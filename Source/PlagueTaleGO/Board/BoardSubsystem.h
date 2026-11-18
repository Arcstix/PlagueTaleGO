// BoardSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "BoardTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "BoardSubsystem.generated.h"

class ABoard;
class ABoardNode;
class ABoardPawn;
class ABoardItemPickup;   
class UBoardItemType;     

// Un USTRUCT est nécessaire : UPROPERTY ne gère pas les conteneurs imbriqués
USTRUCT()
struct FNodeOccupants
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<ABoardPawn>> Entities;
};

// Un objet posé sur une case, et combien de fois la case peut encore le donner
USTRUCT()
struct FNodeArtefact
{
	GENERATED_BODY()

	// Type d'objet (Data Asset)
	UPROPERTY()
	TObjectPtr<UBoardItemType> Type;

	// Visuel affiché sur la case, détruit quand la case est épuisée
	UPROPERTY()
	TObjectPtr<ABoardItemPickup> Pickup;

	// Utilisations restantes (ignoré si bUnlimited)
	int32 RemainingUses = 1;

	// La case donne l'objet à chaque passage
	bool bUnlimited = false;
};

/**
 * Seule autorité sur le plateau en jeu : graphe (cases ABoardNode + liens explicites
 * du layout du ABoard), occupation des cases et validation des déplacements.
 */
UCLASS()
class PLAGUETALEGO_API UBoardSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Construit le graphe. Appelé par le moteur avant les BeginPlay des acteurs.
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// Le Board du niveau (grille + layout), ou nullptr s'il n'y en a pas
	UFUNCTION(BlueprintPure, Category = "Board")
	ABoard* GetBoard() const { return Board; }

	// --- Graphe ---
	UFUNCTION(BlueprintPure, Category = "Board")
	ABoardNode* GetNodeAt(FIntPoint Coord) const;

	// Vrai si un lien a été tracé entre A et B dans le Level Editor
	UFUNCTION(BlueprintPure, Category = "Board")
	bool AreLinked(const ABoardNode* A, const ABoardNode* B) const;

	// Cases reliées à Node par un lien
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

	/**
	 * Cases dans une direction en croix à partir de Origin (Origin non comprise), SANS tenir compte des liens.
	 * Range = nombre de coordonnées parcourues : une coordonnée sans case compte quand même
	 * (un trou ne fait pas gagner de distance).
	 * bStopAtGap : vrai = s'arrête à la première coordonnée sans case ; faux = la saute et continue.
	 * Renvoie les cases dans l'ordre, de la plus proche à la plus lointaine.
	 */
	UFUNCTION(BlueprintCallable, Category = "Board")
	TArray<ABoardNode*> GetNodesInDirection(const ABoardNode* Origin, EBoardDirection Direction, int32 Range, bool bStopAtGap) const;
	
	// --- Objets ---

	// Enregistre un objet posé sur une case au démarrage (appelé par le GameMode)
	void RegisterItem(ABoardNode* Node, UBoardItemType* Type, ABoardItemPickup* Pickup);

	// Vrai si la case donne encore un objet
	UFUNCTION(BlueprintPure, Category = "Board")
	bool HasItemAt(const ABoardNode* Node) const;

	// Prend l'objet de la case : renvoie son type et retire une utilisation (sauf si illimité).
	// À zéro, le visuel est détruit et la case redevient normale. nullptr s'il n'y a pas d'objet.
	UBoardItemType* ConsumeItemAt(ABoardNode* Node);
	
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

	// Objets posés sur les cases (une entrée par case qui donne encore un objet)
	UPROPERTY()
	TMap<TObjectPtr<ABoardNode>, FNodeArtefact> NodeItems;
};