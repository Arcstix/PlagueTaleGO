#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "BoardAudioSubsystem.generated.h"

class UAudioComponent;
class UBoardAudioLibrary;
class UBoardDefinition;
class USceneComponent;
class USoundAttenuation;
class USoundBase;
struct FBoardSoundEntry;

/**
 * The game's sound player: the ONLY entry point for code to play a gameplay sound.
 * Godot equivalent: the AudioManager autoload. A World Subsystem is created and destroyed with the level:
 * no sound survives a level change (team decision: no continuous music).
 *
 * Responsibilities:
 * - find the sound of an event (Audio.* tag): first in the context's SoundOverrides
 *   (cell / pawn / item type), otherwise in the library (UBoardAudioLibrary);
 * - play it in 2D or 3D, as a one-shot or a loop;
 * - keep track of loops so they can all be stopped (victory, defeat).
 *
 * What it does NOT do, because the engine already does it (configured on the assets):
 * max simultaneous sounds (Sound Concurrency), volume categories (Sound Class),
 * audio component recycling, attenuation and spatialization (Sound Attenuation).
 *
 * Usage from C++:
 *   if (UBoardAudioSubsystem* Audio = UBoardAudioSubsystem::Get(this))
 *   {
 *       Audio->PlaySoundAtLocation(AudioTags::Player_Footstep, GetActorLocation(), Node->GetNodeType());
 *   }
 */
UCLASS()
class PLAGUETALEGO_API UBoardAudioSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Shortcut: the subsystem of this object's world (nullptr in the editor outside PIE)
	static UBoardAudioSubsystem* Get(const UObject* WorldContextObject);

	// Only created in game and PIE (not in the editor or previews)
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// --- One-shots (fire and forget: the engine releases the sound by itself when it ends) ---

	// Non-spatialized sound (UI, victory...): same volume whatever the camera
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlaySound2D(FGameplayTag Event, const UBoardDefinition* Context = nullptr);

	// Spatialized sound at a world location (footstep, impact...)
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlaySoundAtLocation(FGameplayTag Event, FVector Location, const UBoardDefinition* Context = nullptr);

	// --- Loops (return the audio component: keep it in a UPROPERTY to stop the loop) ---

	// Non-spatialized loop (ambience, music)
	UFUNCTION(BlueprintCallable, Category = "Audio")
	UAudioComponent* StartLoop2D(FGameplayTag Event, const UBoardDefinition* Context = nullptr);

	// Static spatialized loop. For a scenery sound, prefer an "Ambient Sound" actor placed in the level.
	UFUNCTION(BlueprintCallable, Category = "Audio")
	UAudioComponent* StartLoopAtLocation(FGameplayTag Event, FVector Location, const UBoardDefinition* Context = nullptr);

	// Spatialized loop that follows a component (pawn...) and stops by itself if it is destroyed
	UFUNCTION(BlueprintCallable, Category = "Audio")
	UAudioComponent* StartLoopAttached(FGameplayTag Event, USceneComponent* AttachTo, const UBoardDefinition* Context = nullptr);

	// Stops a loop, with a fade out (0 = hard cut). No effect if Loop is null or already stopped.
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopLoop(UAudioComponent* Loop, float FadeOutDuration = 0.5f);

	// Stops every loop started by this subsystem (victory, defeat...)
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopAllLoops(float FadeOutDuration = 0.5f);

private:
	// Entry to play: context's SoundOverrides, otherwise library. nullptr (+ one warning per tag) if none.
	const FBoardSoundEntry* FindEntry(FGameplayTag Event, const UBoardDefinition* Context);

	// Picks the sound to play and checks the loop / one-shot rule. nullptr if nothing must be played.
	USoundBase* PickSound(const FBoardSoundEntry& Entry, FGameplayTag Event, bool bExpectLoop);

	// Attenuation of a 3D sound: the entry's, otherwise the asset's (nullptr = the engine reads it), otherwise the default one
	USoundAttenuation* ResolveAttenuation(const FBoardSoundEntry& Entry, const USoundBase* Sound) const;

	// Remembers a loop (for StopAllLoops) and returns it
	UAudioComponent* RegisterLoop(UAudioComponent* Loop);

	// Warning shown only once per key (avoids flooding the log on every footstep)
	void WarnOnce(const FString& Key, const FString& Message);

	// --- Data ---

	// Library loaded from the Project Settings (UBoardAudioSettings)
	UPROPERTY()
	TObjectPtr<UBoardAudioLibrary> Library;

	// Running loops. Weak: an audio component destroys itself once its loop is stopped.
	TArray<TWeakObjectPtr<UAudioComponent>> ActiveLoops;

	// Keys of the warnings already shown
	TSet<FString> ReportedWarnings;
};
