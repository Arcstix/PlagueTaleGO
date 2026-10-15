// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerTurnState.h"
#include "EnemyTurnState.h"
#include "ItemInputState.h"


void UPlayerTurnState::PrimaryInput()
{
	nextState = UEnemyTurnState::StaticClass();
}

void UPlayerTurnState::SecondaryInput()
{
	nextState = UItemInputState::StaticClass();
}
