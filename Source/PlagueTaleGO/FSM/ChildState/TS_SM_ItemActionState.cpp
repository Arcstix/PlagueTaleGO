// Fill out your copyright notice in the Description page of Project Settings.



#include "TS_SM_ItemActionState.h"

#include "TS_MS_PlayerInput.h"


void UTS_SM_ItemActionState::PrimaryInput()
{
	nextState = UTS_MS_PlayerInput::StaticClass();
}