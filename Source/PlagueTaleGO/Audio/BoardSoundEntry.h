#pragma once

#include "CoreMinimal.h"
#include "BoardSoundEntry.generated.h"

class USoundBase;
class USoundAttenuation;
class USoundConcurrency;

/**
 * Un son prêt à jouer et ses réglages : une ligne de la bibliothèque (UBoardAudioLibrary),
 * ou une variante propre à un type de case / de pion / d'objet (UBoardDefinition::SoundOverrides).
 * Équivalent Godot : la Resource SoundEffect de l'AudioManager.
 *
 * Ce qui n'est PAS ici, volontairement :
 * - la catégorie de volume (Sound Class) : elle se règle sur l'asset son lui-même
 *   (par défaut SC_SFX, cf. Project Settings > Audio > Default Sound Class) ;
 * - le fait qu'un son boucle : c'est une propriété de l'asset son (case « Looping » d'une Sound Wave).
 */
USTRUCT(BlueprintType)
struct PLAGUETALEGO_API FBoardSoundEntry
{
	GENERATED_BODY()

	// --- Son ---

	// Sons possibles : un est tiré au hasard à chaque lecture (variantes d'un même pas…).
	// USoundBase = n'importe quel son UE : Sound Wave (fichier importé) ou MetaSound Source.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ToolTip = "Possible sounds: one is picked at random each time (variations). Sound Wave or MetaSound Source."))
	TArray<TObjectPtr<USoundBase>> Sounds;

	// Multiplicateur de volume (1 = volume de l'asset)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "4.0", ToolTip = "Volume multiplier (1 = asset volume)."))
	float VolumeMultiplier = 1.f;

	// Multiplicateur de hauteur (1 = hauteur d'origine, 2 = une octave au-dessus)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0.2", ClampMax = "4.0", ToolTip = "Pitch multiplier (1 = original pitch, 2 = one octave higher)."))
	float PitchMultiplier = 1.f;

	// Variation aléatoire ajoutée à la hauteur, de -X à +X à chaque lecture (comme pitch_randomness dans Godot)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "Random pitch variation, from -X to +X each time the sound plays. Avoids repetitive sounds."))
	float PitchRandomness = 0.f;

	// --- Réglages avancés (optionnels) ---

	// Atténuation avec la distance, pour les sons 3D. Vide : celle de l'asset son, sinon celle par défaut de la bibliothèque.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Advanced", meta = (ToolTip = "3D sounds only: how the volume decreases with distance. Empty = the sound asset's attenuation, or the library default."))
	TObjectPtr<USoundAttenuation> Attenuation;

	// Limite de sons simultanés et règle de coupure (équivalent du « limit » de l'AudioManager Godot). Vide : celle de l'asset son.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Advanced", meta = (ToolTip = "Max number of simultaneous instances and what happens when the limit is reached. Empty = the sound asset's concurrency."))
	TObjectPtr<USoundConcurrency> Concurrency;

	// --- Lecture ---

	// Vrai si au moins un son est renseigné
	bool HasSound() const;

	// Un son tiré au hasard parmi ceux renseignés (nullptr si aucun)
	USoundBase* PickRandomSound() const;

	// Hauteur à utiliser pour cette lecture (PitchMultiplier ± PitchRandomness)
	float PickPitch() const;
};
