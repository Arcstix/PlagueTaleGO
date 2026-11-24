#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "BoardSoundEntry.h"
#include "BoardAudioLibrary.generated.h"

class USoundAttenuation;

/**
 * The game's sound library: for each sound event (Audio.* tag), the sound to play.
 * A single asset for the whole project (DA_AudioLibrary), selected in
 * Project Settings > Game > Board Audio > Library (see UBoardAudioSettings).
 * Godot equivalent: the sound_effects array of the AudioManager.
 *
 * Only UBoardAudioSubsystem reads it. A cell / pawn / item type can replace
 * one of these entries for itself (UBoardDefinition::SoundOverrides).
 */
UCLASS(BlueprintType, meta = (ToolTip = "The game's sound library: the sound to play for each sound event (Audio.* tag). One asset for the whole project."))
class PLAGUETALEGO_API UBoardAudioLibrary : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Entry of this event, or nullptr if it has none (or if it is empty)
	const FBoardSoundEntry* FindEntry(FGameplayTag Event) const;

	// --- Sounds ---

	// Sound event -> sound. Categories = "Audio": the dropdown only offers Audio.* tags
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (Categories = "Audio", ToolTip = "Sound event (Audio.* tag) -> sound to play."))
	TMap<FGameplayTag, FBoardSoundEntry> Sounds;

	// --- Defaults ---

	// Attenuation of 3D sounds that have none (neither in their entry nor on their asset).
	// Without it, a "3D" sound would be heard everywhere at the same volume, like a 2D sound.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Defaults", meta = (ToolTip = "Attenuation used by 3D sounds that have none (neither in their entry nor on their asset). Without it, a 3D sound is heard everywhere at the same volume."))
	TObjectPtr<USoundAttenuation> DefaultAttenuation;
};
