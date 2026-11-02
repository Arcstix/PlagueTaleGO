// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Turns/GameFlowState.h"
#include "PlayerMoveState.generated.h"

class ABoardPawn;

UCLASS()
class PLAGUETALEGO_API UPlayerMoveState : public UGameFlowState
{
	GENERATED_BODY()

public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual bool HandleIntent(const FPlayerIntent& Intent) override;

	// Le joueur peut cliquer tant que son déplacement n'a pas commencé
	virtual bool AllowsPlayerInput() const override { return !bMoveInProgress; }

private:
	
	UFUNCTION()
	void HandleMoveFinished(ABoardPawn* MovedPawn);

	bool bMoveInProgress = false;
};
