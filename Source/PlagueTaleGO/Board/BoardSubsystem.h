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

	// Retire un pion du plateau (détruit, mort, capturé…) : libère sa case
	void UnregisterPawn(ABoardPawn* Pawn);

	// Case sur laquelle se trouve un pion (sa case d'arrivée s'il est en mouvement)
	UFUNCTION(BlueprintPure, Category = "Board")
	ABoardNode* GetPawnNode(const ABoardPawn* Pawn) const;

	// Seule autorité : valide, met à jour l'occupation, lance le déplacement
	EMoveResult TryMovePawn(ABoardPawn* Pawn, ABoardNode* InTargetNode);

	UFUNCTION(BlueprintPure, Category = "Board")
	bool IsOccupied(const ABoardNode* Node) const;

	// --- Requêtes ---
	// Pion présent sur la case, ou nullptr si elle est libre
	UFUNCTION(BlueprintPure, Category = "Board")
	ABoardPawn* GetPawnOn(const ABoardNode* Node) const;

	// Vrai si la case est une case de fin
	UFUNCTION(BlueprintPure, Category = "Board")
	bool IsExit(const ABoardNode* Node) const;

	/**
	 * Chemin le plus court de From à To (parcours en largeur), en respectant
	 * les liens bloqués et les cases bloquées.
	 * Renvoie les cases à traverser SANS la case de départ, AVEC l'arrivée :
	 * Path[0] = prochain pas. Tableau vide = pas de chemin (ou From == To).
	 * bAvoidOccupied : contourne les cases occupées (sauf l'arrivée, pour
	 * qu'un ennemi puisse viser la case du joueur).
	 */
	UFUNCTION(BlueprintCallable, Category = "Board")
	TArray<ABoardNode*> FindPath(ABoardNode* From, ABoardNode* To, bool bAvoidOccupied = false) const;

private:
	// Seul endroit qui change la position d'un pion : met à jour
	// PawnToNode ET Occupancy ensemble, pour qu'ils ne divergent jamais
	void SetPawnNode(ABoardPawn* Pawn, ABoardNode* NewNode);
	
	UPROPERTY()
	TObjectPtr<ABoard> Board;

	UPROPERTY()
	TMap<ABoardPawn*, ABoardNode*> PawnToNode;

	UPROPERTY()
	TMap<FIntPoint, TObjectPtr<ABoardNode>> NodesByCoord;
	
	UPROPERTY()
	TMap<TObjectPtr<ABoardNode>, FNodeOccupants> Occupancy;
};