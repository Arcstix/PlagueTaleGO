// Fill out your copyright notice in the Description page of Project Settings.


#include "TS_SM_LooseState.h"

#include "TS_SM_CheckLooseConditionState.h"

#include "TS_SM_CheckPlayerState.h"


void UTS_SM_LooseState::PrimaryInput()
{
	nextState = UTS_SM_CheckPlayerState::StaticClass();
}
