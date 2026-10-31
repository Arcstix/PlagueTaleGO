// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardGameMode.h"
#include "PlagueTaleGO/Turns/StateBase.h"
#include "Engine/Engine.h"
#include "PlagueTaleGO/PlagueTaleGO.h"

void ABoardGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	StartStateMachineOnStart();
	
}

void ABoardGameMode::StartStateMachineOnStart()
{
	if (FirstState)
	{
		
		CurrentState = NewObject<UStateBase>(this, FirstState);
		DebugLogActualState();
		CurrentState->EnterState();
		
	}
}

void ABoardGameMode::DebugLogActualState()
{

	if (!CurrentState) return;
	
	UE_LOG(LogPlagueTaleGO, Log, TEXT("État courant : %s"), *CurrentState->GetName());

#if !UE_BUILD_SHIPPING
	if (CurrentState)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
			CurrentState->GetName());
	}
#endif
}

bool ABoardGameMode::CanPlayerAct() const
{
	return CurrentState && CurrentState->AllowsPlayerInput();
}

void ABoardGameMode::GoToNextState()
{
	if (CurrentState)
	{
		TSubclassOf<UStateBase> nextState = CurrentState->GetNextState();
		if (nextState)
		{
			CurrentState->ExitState();
			CurrentState = NewObject<UStateBase>(this, nextState);
			CurrentState->EnterState();
		
			DebugLogActualState();
		}
		
		
	}
}
