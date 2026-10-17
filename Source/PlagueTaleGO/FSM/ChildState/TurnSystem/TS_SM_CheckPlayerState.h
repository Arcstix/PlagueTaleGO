// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/FSM/StateBase.h"
#include "TS_SM_CheckPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class PLAGUETALEGO_API UTS_SM_CheckPlayerState : public UStateBase
{
	GENERATED_BODY()
	
	public:
	

		void PrimaryInput() override;
	
		void SecondaryInput() override;
	
	
};
