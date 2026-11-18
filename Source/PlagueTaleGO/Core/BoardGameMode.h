// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BoardGameMode.generated.h"

class APlayerPawn;
class UBoardSubsystem;
class ABoardItemPickup;
struct FBoardCell;
struct FBoardEntityPlacement;
struct FBoardItemPlacement;

/**
 * GameMode des niveaux de plateau : crée les pions de départ décrits par le layout
 * du ABoard, puis lance la machine à états (UGameFlowSubsystem).
 */
UCLASS()
class PLAGUETALEGO_API ABoardGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABoardGameMode();
	// Lance la partie : fait apparaître les pions depuis les cases du niveau
	virtual void StartPlay() override;

	UFUNCTION(BlueprintPure, Category = "Board")
	APlayerPawn* GetPlayerPawn() const { return PlayerPawn; }

protected:
	// Classe du visuel des objets posés. Remplaçable par un BP enfant (animation…).
	UPROPERTY(EditDefaultsOnly, Category = "Board", meta = (ToolTip = "Actor class used to show items placed on cells."))
	TSubclassOf<ABoardItemPickup> ItemPickupClass;

private:
	// Parcourt les cases du layout et crée chaque entité de départ sur sa case
	void SpawnBoardEntities();

	// Crée un pion de départ, lui transmet ses réglages et l'enregistre sur sa case
	void SpawnEntity(const FBoardCell& Cell, const FBoardEntityPlacement& Placement, UBoardSubsystem& BoardSubsystem);

	// Crée le visuel d'un objet posé sur une case
	void SpawnItemPickup(const FBoardCell& Cell, const FBoardItemPlacement& Placement, UBoardSubsystem& BoardSubsystem);
	
	// Le pion joueur créé au démarrage
	UPROPERTY()
	TObjectPtr<APlayerPawn> PlayerPawn;
	
};
