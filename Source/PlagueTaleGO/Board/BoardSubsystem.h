// BoardSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "BoardTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "BoardSubsystem.generated.h"

class ABoard;
class ABoardNode;
class ABoardPawn;

// Un USTRUCT est nécessaire : UPROPERTY ne gère pas les conteneurs imbriqués
USTRUCT()
struct FNodeOccupants
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<ABoardPawn>> Entities;
};

/**
 * Seule autorité sur le plateau : graphe (calculé depuis la grille),
 * occupation des cases et validation des déplacements.
 */
UCLASS()
class PLAGUETALEGO_API UBoardSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Construit le graphe. Appelé par le moteur avant les BeginPlay des acteurs.
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// --- Graphe ---
	UFUNCTION(BlueprintPure, Category = "Board")
	ABoardNode* GetNodeAt(FIntPoint Coord) const;

	// Vrai si A et B sont voisines sur la grille ET que le lien n'est pas bloqué
	UFUNCTION(BlueprintPure, Category = "Board")
	bool AreLinked(const ABoardNode* A, const ABoardNode* B) const;

	UFUNCTION(BlueprintPure, Category = "Board")
	TArray<ABoardNode*> GetNeighbors(const ABoardNode* Node) const;
	
	// Enregistre une entité présente sur un nœud au démarrage
	void RegisterPawn(ABoardPawn* Pawn, ABoardNode* Node);

	// Seule autorité : valide, met à jour l'occupation, lance le déplacement
	EMoveResult TryMovePawn(ABoardPawn* Pawn, ABoardNode* InTargetNode);

	UFUNCTION(BlueprintPure, Category = "Board")
	bool IsOccupied(const ABoardNode* Node) const;

private:
	UPROPERTY()
	TObjectPtr<ABoard> Board;

	UPROPERTY()
	TMap<FIntPoint, TObjectPtr<ABoardNode>> NodesByCoord;
	
	UPROPERTY()
	TMap<TObjectPtr<ABoardNode>, FNodeOccupants> Occupancy;
};