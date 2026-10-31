// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PlagueTaleGO/Turns/StateBase.h"
#include "BoardGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PLAGUETALEGO_API ABoardGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
	protected:
	UPROPERTY()
	TObjectPtr<UStateBase> CurrentState;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UStateBase> FirstState;
	
	virtual void BeginPlay() override;
	
	public:
	void StartStateMachineOnStart();
	void DebugLogActualState();
	bool CanPlayerAct() const;
	void GoToNextState();
	
};
