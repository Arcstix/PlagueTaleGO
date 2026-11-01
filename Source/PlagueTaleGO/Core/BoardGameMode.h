// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
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

private:
	// Parcourt les cases et crée chaque StartingEntity sur sa case
	void SpawnBoardEntities();
	
	// Le pion joueur créé au démarrage
	UPROPERTY()
	TObjectPtr<APlayerPawn> PlayerPawn;
	
};
