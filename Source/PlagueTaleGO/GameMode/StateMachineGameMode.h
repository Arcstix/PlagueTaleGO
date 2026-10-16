// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PlagueTaleGO/FSM/StateBase.h"
#include "StateMachineGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PLAGUETALEGO_API AStateMachineGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
	protected:
	UPROPERTY()
	TObjectPtr<UStateBase> actualState;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UStateBase> firstState;
	
	virtual void BeginPlay() override;
	
	public:
	void StartStateMachineOnStart();
	void DebugLogActualState();
	bool CanPlayerAct() const;
	void GoToNextState();
	
};
