// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PlagueTaleGO/Turns/StateBase.h"
#include "BoardGameMode.generated.h"

class APlayerPawn;

/**
 * 
 */
UCLASS()
class PLAGUETALEGO_API ABoardGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// Lance la partie : fait apparaître les pions depuis les cases du niveau
	virtual void StartPlay() override;

	UFUNCTION(BlueprintPure, Category = "Board")
	APlayerPawn* GetPlayerPawn() const { return PlayerPawn; }

	void StartStateMachineOnStart();
	void DebugLogActualState();
	bool CanPlayerAct() const;
	void GoToNextState();
	
protected:
	UPROPERTY()
	TObjectPtr<UStateBase> CurrentState;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UStateBase> FirstState;
	
	virtual void BeginPlay() override;

private:
	// Parcourt les cases et crée chaque StartingEntity sur sa case
	void SpawnBoardEntities();
	
	// Le pion joueur créé au démarrage (servira à la tâche 25)
	UPROPERTY()
	TObjectPtr<APlayerPawn> PlayerPawn;
	
};
