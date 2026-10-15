// Fill out your copyright notice in the Description page of Project Settings.


#include "StateBase.h"

void UStateBase::EnterState()
{
}

void UStateBase::ExitState()
{
}

TSubclassOf<UStateBase> UStateBase::GetNextState()
{
	return nextState;
}

void UStateBase::PrimaryInput()
{
}

void UStateBase::SecondaryInput()
{
}
