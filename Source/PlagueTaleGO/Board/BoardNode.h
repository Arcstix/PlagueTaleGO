	// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlagueTaleGO/Items/BoardItem.h"
#include "BoardNode.generated.h"

	class ABoardPawn;
class UStaticMeshComponent;
class ABoardConnection;
class ABoard;

/**
 * Une case du plateau. Elle se cale sur la grille du ABoard du niveau.
 * Elle ne connaît PAS ses voisins : c'est UBoardSubsystem qui les calcule
 * à partir des coordonnées et des liens bloqués du ABoard.
 */
UCLASS()
class PLAGUETALEGO_API ABoardNode : public AActor
{
	GENERATED_BODY()

public:
	ABoardNode();

	// --- Lecture (utilisée par le subsystem, le GameMode, les états) ---
	UFUNCTION(BlueprintPure, Category = "Board")
	FIntPoint GetGridCoord() const { return GridCoord; }

	UFUNCTION(BlueprintPure, Category = "Board")
	bool IsBlocked() const { return bIsBlocked; }

	UFUNCTION(BlueprintPure, Category = "Board")
	bool IsExit() const { return bIsExit; }

	TSubclassOf<ABoardPawn> GetStartingEntity() const { return StartingEntity; }

	TSubclassOf<UBoardItem> GetItem() const { return Item; }
	bool HasItem() const { return Item != nullptr; }

	// Appelé quand le joueur ramasse l'objet (en jeu uniquement)
	// void ClearItem() { Item = nullptr; }

	void SetBlocked(bool bValue) { bIsBlocked = bValue; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	// Redessine les liens quand on déplace la case dans l'éditeur
	virtual void PostEditMove(bool bFinished) override;
#endif

	// --- Données du niveau (renseignées par le level designer / l'outil) ---

	// Coordonnées sur la grille, calculées automatiquement : non éditable
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Board")
	FIntPoint GridCoord = FIntPoint::ZeroValue;

	// Pion créé sur cette case au démarrage (joueur ou ennemi). Vide = aucun.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Board|Level")
	TSubclassOf<ABoardPawn> StartingEntity;

	// Objet posé sur cette case. Vide = aucun.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Board|Level")
	TSubclassOf<UBoardItem> Item;

	// Atteindre cette case fait gagner la partie
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Board|Level")
	bool bIsExit = false;

	// Case infranchissable (obstacle temporaire, effet d'objet…)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Board|Level")
	bool bIsBlocked = false;

private:
	// Le ABoard du niveau, ou nullptr s'il n'y en a pas
	ABoard* FindBoard() const;
};
