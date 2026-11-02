// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Turns/GameFlowState.h"
#include "ItemUseState.generated.h"

/**
 * 
 */
UCLASS()
class PLAGUETALEGO_API UItemUseState : public UGameFlowState
{
	GENERATED_BODY()

public:
	virtual void Enter() override;
};
