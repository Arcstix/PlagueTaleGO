// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyTurnState.h"
#include "ItemInputState.h"
#include "PlayerTurnState.h"



void UEnemyTurnState::PrimaryInput()
{
	nextState = UPlayerTurnState::StaticClass();
}

void UEnemyTurnState::SecondaryInput()
{
	nextState = UItemInputState::StaticClass();
}