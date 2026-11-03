// Fill out your copyright notice in the Description page of Project Settings.


#include "StateMachineGameMode.h"
#include "PlagueTaleGO/FSM/StateBase.h"
#include "Engine/Engine.h"

void AStateMachineGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	StartStateMachineOnStart();
	
}

void AStateMachineGameMode::StartStateMachineOnStart()
{
	if (firstState)
	{
		
		actualState = NewObject<UStateBase>(this, firstState);
		DebugLogActualState();
		actualState->EnterState();
		
	}
}

void AStateMachineGameMode::DebugLogActualState()
{
	if (actualState)
	{
		// Efface les logs de l'état précédent
		GEngine->ClearOnScreenDebugMessages();

		// Etat "sonde" : copie jetable de l'état actuel, pour simuler un input sans toucher au vrai état
		UStateBase* primaryProbe = NewObject<UStateBase>(this, actualState->GetClass());
		primaryProbe->PrimaryInput();

		UStateBase* secondaryProbe = NewObject<UStateBase>(this, actualState->GetClass());
		secondaryProbe->SecondaryInput();

		// Affichés en premier pour que l'état actuel apparaisse au-dessus
		GEngine->AddOnScreenDebugMessage(-1, 120.0f, FColor::Cyan,
			FString::Printf(TEXT("  [E] Secondary Input -> %s"), *GetNameSafe(secondaryProbe->GetNextState())));
		GEngine->AddOnScreenDebugMessage(-1, 120.0f, FColor::Green,
			FString::Printf(TEXT("  [Space] Primary Input -> %s"), *GetNameSafe(primaryProbe->GetNextState())));
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red,
			FString::Printf(TEXT("Etat actuel : %s"), *actualState->GetName()));
	}
}

void AStateMachineGameMode::GoToNextState()
{
	if (actualState)
	{
		TSubclassOf<UStateBase> nextState = actualState->GetNextState();
		if (nextState)
		{
			actualState->ExitState();
			actualState = NewObject<UStateBase>(this, nextState);
			actualState->EnterState();
		
			DebugLogActualState();
		}
		
		
	}
}

UStateBase* AStateMachineGameMode::GetActualState() const
{
	return actualState;
}

bool AStateMachineGameMode::CanPlayerAct() const
{
	return actualState && actualState->AllowsPlayerInput();
}
