// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PlagueTaleGO/Board/BoardTypes.h"
#include "BoardItem.generated.h"

class ABoardNode;
class ABoardPawn;
class UBoardSubsystem;

// Ce que l'objet attend du joueur au moment de l'utiliser
UENUM(BlueprintType)
enum class EBoardItemTargeting : uint8
{
	None		UMETA(DisplayName = "None"),		// utilisé directement, sans choix
	Direction	UMETA(DisplayName = "Direction")	// le joueur choisit une direction en croix
};

/**
 * Tout ce dont un objet a besoin pour s'utiliser.
 * Rempli par UItemUseState au moment où le joueur valide son choix.
 */
USTRUCT(BlueprintType)
struct FBoardItemUseContext
{
	GENERATED_BODY()

	// Le plateau : pour trouver les cases touchées (l'objet ne cherche pas lui-même dans le monde)
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	TObjectPtr<UBoardSubsystem> Board = nullptr;

	// Le pion qui utilise l'objet (le joueur)
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	TObjectPtr<ABoardPawn> User = nullptr;

	// La case d'où l'objet est utilisé
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	TObjectPtr<ABoardNode> OriginNode = nullptr;

	// Direction choisie (seulement si Targeting == Direction)
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	EBoardDirection Direction = EBoardDirection::North;
};

/**
 * Un choix possible pour un objet directionnel : une direction et les cases qu'elle toucherait.
 * Une direction n'est proposée que si elle touche au moins une case.
 */
USTRUCT(BlueprintType)
struct FBoardDirectionOption
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	EBoardDirection Direction = EBoardDirection::North;

	// Cases touchées, de la plus proche à la plus lointaine (jamais vide)
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	TArray<TObjectPtr<ABoardNode>> Nodes;
};

/**
 * Base de tous les objets du jeu.
 * Un objet est créé quand le joueur le prend sur une case (PendingItem de UGameFlowSubsystem),
 * puis utilisé au début de son tour suivant par UItemUseState.
 * Responsabilités : dire comment il se cible, calculer les cases touchées, appliquer son effet.
 * Il ne change JAMAIS l'occupation du plateau lui-même : s'il doit déplacer un pion, il passe par UBoardSubsystem.
 */
UCLASS(Abstract, Blueprintable)
class PLAGUETALEGO_API UBoardItem : public UObject
{
	GENERATED_BODY()

public:
	// --- Ciblage ---

	// Ce que le joueur doit choisir avant l'utilisation
	UFUNCTION(BlueprintPure, Category = "Item")
	EBoardItemTargeting GetTargeting() const { return Targeting; }

	// --- Utilisation ---

	// Calcule et renvoie les cases touchées. Ne modifie rien : l'effet est appliqué par ApplyToNode.
	// La base ne touche aucune case : chaque objet concret la surcharge.
	virtual TArray<ABoardNode*> Use(const FBoardItemUseContext& Context);

	// Effet de jeu sur une case touchée. Pas encore défini : vide pour l'instant.
	virtual void ApplyToNode(ABoardNode* Node) {}

	// --- Choix (objets directionnels) ---

	// Directions valides et leurs cases : celles qui touchent au moins une case.
	// BaseContext fournit le plateau, l'utilisateur et sa case ; la direction est remplie ici.
	// Vide si l'objet ne se cible pas par direction, ou si aucune direction ne touche de case.
	TArray<FBoardDirectionOption> GetDirectionOptions(const FBoardItemUseContext& BaseContext);

	// Retrouve la direction dont fait partie une case cliquée. Faux si la case n'est dans aucun choix.
	static bool FindDirectionOfNode(const TArray<FBoardDirectionOption>& Options, const ABoardNode* Node, EBoardDirection& OutDirection);
	
protected:
	// Fixé dans le constructeur de chaque objet concret (pas modifiable dans un BP : c'est la nature de l'objet)
	UPROPERTY(VisibleDefaultsOnly, Category = "Item")
	EBoardItemTargeting Targeting = EBoardItemTargeting::None;
};
