#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "BoardAudioSettings.generated.h"

class UBoardAudioLibrary;

/**
 * Project audio settings, shown in Project Settings > Game > Board Audio.
 *
 * UDeveloperSettings: a class whose "Config" properties automatically appear
 * in the Project Settings and are saved in Config/DefaultGame.ini (DefaultConfig).
 * Godot equivalent: declaring an autoload in Project Settings, without writing code.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Board Audio"))
class PLAGUETALEGO_API UBoardAudioSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UBoardAudioSettings();

	// Sound library used by UBoardAudioSubsystem.
	// "Soft" reference (a path): required for a Config property, the asset is loaded when the level starts.
	UPROPERTY(Config, EditAnywhere, Category = "Audio", meta = (ToolTip = "Sound library used by the game (DA_AudioLibrary)."))
	TSoftObjectPtr<UBoardAudioLibrary> Library;
};
