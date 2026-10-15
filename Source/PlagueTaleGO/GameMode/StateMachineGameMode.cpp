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
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
			actualState->GetName());
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
