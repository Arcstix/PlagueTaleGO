// Fill out your copyright notice in the Description page of Project Settings.


#include "TS_SM_EnemyCheckTile.h"

#include "TS_SM_PlayerMove.h"

#include "TS_SM_CheckPlayerState.h"


void UTS_SM_EnemyCheckTile::PrimaryInput()
{
	nextState = UTS_SM_CheckPlayerState::StaticClass();
}
