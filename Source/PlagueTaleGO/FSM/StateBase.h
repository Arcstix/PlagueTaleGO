// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "StateBase.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class PLAGUETALEGO_API UStateBase : public UObject
{
	GENERATED_BODY()
	
	public:
	virtual void EnterState();
	virtual void ExitState();
	virtual TSubclassOf<UStateBase> GetNextState();
	virtual bool AllowsPlayerInput() const { return false; }
	
	virtual void PrimaryInput();
	virtual void SecondaryInput();
	
	protected:
		UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UStateBase> nextState;
	
};
