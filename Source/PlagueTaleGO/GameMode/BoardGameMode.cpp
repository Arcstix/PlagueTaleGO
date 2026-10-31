// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardGameMode.h"
#include "PlagueTaleGO/FSM/StateBase.h"
#include "Engine/Engine.h"

void ABoardGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	StartStateMachineOnStart();
	
}

void ABoardGameMode::StartStateMachineOnStart()
{
	if (firstState)
	{
		
		actualState = NewObject<UStateBase>(this, firstState);
		DebugLogActualState();
		actualState->EnterState();
		
	}
}

void ABoardGameMode::DebugLogActualState()
{
	if (actualState)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
			actualState->GetName());
	}
}

bool ABoardGameMode::CanPlayerAct() const
{
	return actualState && actualState->AllowsPlayerInput();
}

void ABoardGameMode::GoToNextState()
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
