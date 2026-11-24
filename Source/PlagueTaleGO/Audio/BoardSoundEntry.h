#pragma once

#include "CoreMinimal.h"
#include "BoardSoundEntry.generated.h"

class USoundBase;
class USoundAttenuation;
class USoundConcurrency;

/**
 * A sound ready to play and its settings: one row of the library (UBoardAudioLibrary),
 * or a variant specific to a cell / pawn / item type (UBoardDefinition::SoundOverrides).
 * Godot equivalent: the SoundEffect Resource of the AudioManager.
 *
 * What is deliberately NOT here:
 * - the volume category (Sound Class): it is set on the sound asset itself
 *   (SC_SFX by default, see Project Settings > Audio > Default Sound Class);
 * - whether a sound loops: it is a property of the sound asset ("Looping" checkbox of a Sound Wave).
 */
USTRUCT(BlueprintType)
struct PLAGUETALEGO_API FBoardSoundEntry
{
	GENERATED_BODY()

	// --- Sound ---

	// Possible sounds: one is picked at random each time (variations of the same footstep...).
	// USoundBase = any UE sound: Sound Wave (imported file) or MetaSound Source.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ToolTip = "Possible sounds: one is picked at random each time (variations). Sound Wave or MetaSound Source."))
	TArray<TObjectPtr<USoundBase>> Sounds;

	// Volume multiplier (1 = asset volume)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "4.0", ToolTip = "Volume multiplier (1 = asset volume)."))
	float VolumeMultiplier = 1.f;

	// Pitch multiplier (1 = original pitch, 2 = one octave higher)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0.2", ClampMax = "4.0", ToolTip = "Pitch multiplier (1 = original pitch, 2 = one octave higher)."))
	float PitchMultiplier = 1.f;

	// Random variation added to the pitch, from -X to +X each time (like pitch_randomness in Godot)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "Random pitch variation, from -X to +X each time the sound plays. Avoids repetitive sounds."))
	float PitchRandomness = 0.f;

	// --- Advanced settings (optional) ---

	// Distance attenuation, for 3D sounds. Empty: the sound asset's own, otherwise the library default.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Advanced", meta = (ToolTip = "3D sounds only: how the volume decreases with distance. Empty = the sound asset's attenuation, or the library default."))
	TObjectPtr<USoundAttenuation> Attenuation;

	// Max simultaneous instances and culling rule (equivalent of "limit" in the Godot AudioManager). Empty: the sound asset's own.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Advanced", meta = (ToolTip = "Max number of simultaneous instances and what happens when the limit is reached. Empty = the sound asset's concurrency."))
	TObjectPtr<USoundConcurrency> Concurrency;

	// --- Playback ---

	// True if at least one sound is set
	bool HasSound() const;

	// A sound picked at random among the ones set (nullptr if none)
	USoundBase* PickRandomSound() const;

	// Pitch to use for this playback (PitchMultiplier ± PitchRandomness)
	float PickPitch() const;
};
