// Fill out your copyright notice in the Description page of Project Settings.


#include "TS_MS_PlayerInput.h"
#include "TS_SM_ItemActionState.h"
#include "TS_SM_PlayerMove.h"


void UTS_MS_PlayerInput::PrimaryInput()
{
	nextState = UTS_SM_PlayerMove::StaticClass();
}
