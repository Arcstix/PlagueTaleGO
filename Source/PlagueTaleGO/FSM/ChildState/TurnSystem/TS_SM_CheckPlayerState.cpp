// Fill out your copyright notice in the Description page of Project Settings.


#include "TS_SM_CheckPlayerState.h"
#include "TS_MS_PlayerInput.h"
#include "TS_SM_ItemInputState.h"


void UTS_SM_CheckPlayerState::PrimaryInput()
{
	nextState = UTS_MS_PlayerInput::StaticClass();
}

void UTS_SM_CheckPlayerState::SecondaryInput()
{
	nextState = UTS_SM_ItemInputState::StaticClass();
}
