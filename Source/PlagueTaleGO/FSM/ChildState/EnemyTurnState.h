// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../StateBase.h"
#include "EnemyTurnState.generated.h"

/**
 * 
 */
UCLASS()
class PLAGUETALEGO_API UEnemyTurnState : public UStateBase
{
	GENERATED_BODY()
	
	public:
	
		void PrimaryInput() override;
	
		void SecondaryInput() override;
	
};
