// Fill out your copyright notice in the Description page of Project Settings.



#include "ItemInputState.h"

#include "EnemyTurnState.h"
#include "PlayerTurnState.h"


void UItemInputState::PrimaryInput()
{
	nextState = UPlayerTurnState::StaticClass();
}

void UItemInputState::SecondaryInput()
{
	nextState = UEnemyTurnState::StaticClass();
}