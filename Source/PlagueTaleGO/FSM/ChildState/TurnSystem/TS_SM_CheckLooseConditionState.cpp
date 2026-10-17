// Fill out your copyright notice in the Description page of Project Settings.


#include "TS_SM_CheckLooseConditionState.h"

#include "TS_SM_CheckPlayerState.h"
#include "TS_SM_LooseState.h"


void UTS_SM_CheckLooseConditionState::PrimaryInput()
{
	nextState = UTS_SM_CheckPlayerState::StaticClass();
}

void UTS_SM_CheckLooseConditionState::SecondaryInput()
{
	nextState = UTS_SM_LooseState::StaticClass();
}
