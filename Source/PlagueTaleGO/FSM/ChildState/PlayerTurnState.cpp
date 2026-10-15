// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerTurnState.h"
#include "EnemyTurnState.h"

TSubclassOf<UStateBase> UPlayerTurnState::GetNextState()
{
	return UEnemyTurnState::StaticClass();
}
