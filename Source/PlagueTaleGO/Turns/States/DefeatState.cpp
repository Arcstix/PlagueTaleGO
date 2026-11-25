// Fill out your copyright notice in the Description page of Project Settings.


#include "DefeatState.h"
#include "Engine/Engine.h"
#include "PlagueTaleGO/PlagueTaleGO.h"
#include "PlagueTaleGO/Audio/AudioTags.h"
#include "PlagueTaleGO/Audio/BoardAudioSubsystem.h"

void UDefeatState::Enter()
{
	UE_LOG(LogPlagueTaleGO, Log, TEXT("[GameFlow] Défaite !"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("DEFAITE"));
	}
	if (UBoardAudioSubsystem* Audio = UBoardAudioSubsystem::Get(this))
	{
		Audio->PlaySound2D(AudioTags::Game_Defeat);
	}
	// Plus tard : widget de fin, niveau suivant…
}
