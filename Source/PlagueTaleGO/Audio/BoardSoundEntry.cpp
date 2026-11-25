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
	// On ignore les cases vides de la liste (oubli fréquent quand on ajoute une ligne)
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
	// Plancher : en dessous, le moteur coupe ou déforme le son
	return FMath::Max(0.2f, PitchMultiplier + FMath::FRandRange(-PitchRandomness, PitchRandomness));
}
