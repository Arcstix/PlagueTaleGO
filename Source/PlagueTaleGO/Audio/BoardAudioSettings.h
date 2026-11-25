#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "BoardAudioSettings.generated.h"

class UBoardAudioLibrary;

/**
 * Réglages audio du projet, affichés dans Project Settings > Game > Board Audio.
 *
 * UDeveloperSettings : une classe dont les propriétés « Config » apparaissent automatiquement
 * dans les Project Settings et sont enregistrées dans Config/DefaultGame.ini (DefaultConfig).
 * Équivalent Godot : déclarer un autoload dans Project Settings, sans écrire de code.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Board Audio"))
class PLAGUETALEGO_API UBoardAudioSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UBoardAudioSettings();

	// Bibliothèque de sons utilisée par UBoardAudioSubsystem.
	// Référence « soft » (un chemin) : obligatoire pour une propriété Config, l'asset est chargé au lancement du niveau.
	UPROPERTY(Config, EditAnywhere, Category = "Audio", meta = (ToolTip = "Sound library used by the game (DA_AudioLibrary)."))
	TSoftObjectPtr<UBoardAudioLibrary> Library;
};
