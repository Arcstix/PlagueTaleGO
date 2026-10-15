// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerTurnState.h"
#include "EnemyTurnState.h"
#include "ItemInputState.h"

TSubclassOf<UStateBase> UPlayerTurnState::GetNextState()
{
	return nextState;
}

void UPlayerTurnState::PrimaryInput()
{
	nextState = UEnemyTurnState::StaticClass();
}

void UPlayerTurnState::SecondaryInput()
{
	nextState = UItemInputState::StaticClass();
}
