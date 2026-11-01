// Fill out your copyright notice in the Description page of Project Settings.


#include "VictoryState.h"
#include "Engine/Engine.h"
#include "PlagueTaleGO/PlagueTaleGO.h"

void UVictoryState::Enter()
{
	UE_LOG(LogPlagueTaleGO, Log, TEXT("[GameFlow] Victoire !"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("VICTOIRE"));
	}
	// Plus tard : widget de fin, niveau suivant…
}
