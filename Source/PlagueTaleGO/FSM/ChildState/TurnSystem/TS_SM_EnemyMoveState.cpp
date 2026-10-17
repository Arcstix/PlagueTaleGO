// Fill out your copyright notice in the Description page of Project Settings.

#include "TS_SM_EnemyMoveState.h"

#include "TS_SM_CheckLooseConditionState.h"


void UTS_SM_EnemyMoveState::PrimaryInput()
{
	nextState = UTS_SM_CheckLooseConditionState::StaticClass();
}


