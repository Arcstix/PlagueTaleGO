// Fill out your copyright notice in the Description page of Project Settings.


#include "TS_SM_ItemInputState.h"

#include "TS_SM_ItemActionState.h"


void UTS_SM_ItemInputState::PrimaryInput()
{
	nextState = UTS_SM_ItemActionState::StaticClass();
}
