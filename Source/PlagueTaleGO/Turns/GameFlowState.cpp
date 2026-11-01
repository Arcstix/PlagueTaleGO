// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFlowState.h"
#include "GameFlowSubsystem.h"

void UGameFlowState::Initialize(UGameFlowSubsystem* InOwner)
{
	Owner = InOwner;
}

void UGameFlowState::Enter()
{
}

void UGameFlowState::Exit()
{
}

UWorld* UGameFlowState::GetWorld() const
{
	return Owner ? Owner->GetWorld() : nullptr;
}

UGameFlowSubsystem* UGameFlowState::GetOwner() const
{
	return Owner;
}

void UGameFlowState::Finish()
{
	if (Owner)
	{
		Owner->NotifyStateFinished(this);
	}
}
