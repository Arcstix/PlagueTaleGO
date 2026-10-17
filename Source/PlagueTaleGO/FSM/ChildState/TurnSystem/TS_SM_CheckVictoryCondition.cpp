// Fill out your copyright notice in the Description page of Project Settings.




#include "TS_SM_PlayerMove.h"

#include "TS_SM_CheckPlayerState.h"
#include "TS_SM_CheckVictoryCondition.h"


void UTS_SM_CheckVictoryCondition::PrimaryInput()
{
	nextState = UTS_SM_CheckPlayerState::StaticClass();
}
