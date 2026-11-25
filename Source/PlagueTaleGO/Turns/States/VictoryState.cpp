// Fill out your copyright notice in the Description page of Project Settings.


#include "VictoryState.h"
#include "Engine/Engine.h"
#include "PlagueTaleGO/PlagueTaleGO.h"
#include "PlagueTaleGO/Audio/AudioTags.h"
#include "PlagueTaleGO/Audio/BoardAudioSubsystem.h"

void UVictoryState::Enter()
{
	UE_LOG(LogPlagueTaleGO, Log, TEXT("[GameFlow] Victoire !"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("VICTOIRE"));
	}
	if (UBoardAudioSubsystem* Audio = UBoardAudioSubsystem::Get(this))
	{
		Audio->PlaySound2D(AudioTags::Game_Victory);
	}
	// Plus tard : widget de fin, niveau suivant…
}
