// Fill out your copyright notice in the Description page of Project Settings.



#include "ItemInputState.h"
#include "PlayerTurnState.h"
TSubclassOf<UStateBase> UItemInputState::GetNextState()
{
	return UPlayerTurnState::StaticClass();
}