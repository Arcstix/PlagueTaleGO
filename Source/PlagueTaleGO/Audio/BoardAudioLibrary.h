#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "BoardSoundEntry.h"
#include "BoardAudioLibrary.generated.h"

class USoundAttenuation;

/**
 * La bibliothèque des sons du jeu : pour chaque événement sonore (tag Audio.*), le son à jouer.
 * Un seul asset pour tout le projet (DA_AudioLibrary), choisi dans
 * Project Settings > Game > Board Audio > Library (voir UBoardAudioSettings).
 * Équivalent Godot : le tableau sound_effects de l'AudioManager.
 *
 * Seul UBoardAudioSubsystem la lit. Un type de case / de pion / d'objet peut remplacer
 * une de ces entrées pour lui-même (UBoardDefinition::SoundOverrides).
 */
UCLASS(BlueprintType, meta = (ToolTip = "The game's sound library: the sound to play for each sound event (Audio.* tag). One asset for the whole project."))
class PLAGUETALEGO_API UBoardAudioLibrary : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Entrée de cet événement, ou nullptr s'il n'en a pas (ou si elle est vide)
	const FBoardSoundEntry* FindEntry(FGameplayTag Event) const;

	// --- Sons ---

	// Événement sonore → son. Categories = "Audio" : la liste déroulante ne propose que les tags Audio.*
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (Categories = "Audio", ToolTip = "Sound event (Audio.* tag) -> sound to play."))
	TMap<FGameplayTag, FBoardSoundEntry> Sounds;

	// --- Réglages par défaut ---

	// Atténuation des sons 3D qui n'en ont aucune (ni dans leur entrée, ni sur leur asset).
	// Sans elle, un son « 3D » serait entendu partout au même volume, comme un son 2D.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Defaults", meta = (ToolTip = "Attenuation used by 3D sounds that have none (neither in their entry nor on their asset). Without it, a 3D sound is heard everywhere at the same volume."))
	TObjectPtr<USoundAttenuation> DefaultAttenuation;
};
