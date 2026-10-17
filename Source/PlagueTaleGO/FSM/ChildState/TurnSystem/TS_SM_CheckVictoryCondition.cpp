// Fill out your copyright notice in the Description page of Project Settings.




#include "TS_SM_PlayerMove.h"

#include "TS_SM_CheckPlayerState.h"
#include "TS_SM_CheckVictoryCondition.h"

#include "TS_SM_EnemyCheckTile.h"
#include "TS_SM_VictoryState.h"


void UTS_SM_CheckVictoryCondition::PrimaryInput()
{
	nextState = UTS_SM_EnemyCheckTile::StaticClass();
}

void UTS_SM_CheckVictoryCondition::SecondaryInput()
{
	nextState = UTS_SM_VictoryState::StaticClass();
}
