// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../StateBase.h"
#include "TS_MS_PlayerInput.generated.h"

/**
 * 
 */
UCLASS()
class PLAGUETALEGO_API UTS_MS_PlayerInput : public UStateBase
{
	GENERATED_BODY()
	
	public:
	
		void PrimaryInput() override;
	
};
