// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyTurnState.h"
#include "ItemInputState.h"

TSubclassOf<UStateBase> UEnemyTurnState::GetNextState()
{
	return UItemInputState::StaticClass();
}