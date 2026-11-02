// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Turns/GameFlowState.h"
#include "VictoryState.generated.h"

/**
 * État final : n'appelle jamais Finish(), n'accepte plus aucune entrée
 */
UCLASS()
class PLAGUETALEGO_API UVictoryState : public UGameFlowState
{
	GENERATED_BODY()

public:
	virtual void Enter() override;
};
