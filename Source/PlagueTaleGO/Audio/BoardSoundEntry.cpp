#include "BoardSoundEntry.h"
#include "Sound/SoundBase.h"

bool FBoardSoundEntry::HasSound() const
{
	for (const TObjectPtr<USoundBase>& Sound : Sounds)
	{
		if (Sound)
		{
			return true;
		}
	}
	return false;
}

USoundBase* FBoardSoundEntry::PickRandomSound() const
{
	// Skip empty slots in the list (common mistake when adding a row)
	TArray<USoundBase*, TInlineAllocator<8>> Valid;
	for (const TObjectPtr<USoundBase>& Sound : Sounds)
	{
		if (Sound)
		{
			Valid.Add(Sound.Get());
		}
	}
	if (Valid.IsEmpty())
	{
		return nullptr;
	}
	return Valid[FMath::RandRange(0, Valid.Num() - 1)];
}

float FBoardSoundEntry::PickPitch() const
{
	// Lower bound: below it, the engine cuts or distorts the sound
	return FMath::Max(0.2f, PitchMultiplier + FMath::FRandRange(-PitchRandomness, PitchRandomness));
}
