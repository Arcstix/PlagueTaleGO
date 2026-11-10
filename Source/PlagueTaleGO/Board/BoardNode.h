	// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "BoardNode.generated.h"

class ABoardPawn;
class UBoardItem;
class UStaticMeshComponent;
class UBoardNodeType;
class ABoard;

/**
 * Une case du plateau, telle qu'elle existe dans la scène.
 *
 * Les cases sont GÉNÉRÉES par le Level Editor à partir du layout du ABoard (Submit) :
 * on ne les place pas et on ne les règle pas à la main.
 * - Son visuel et ses comportements viennent de son type (UBoardNodeType).
 * - Elle ne connaît PAS ses voisines : UBoardSubsystem les calcule à partir
 *   des liens explicites du layout.
 * - Les entités et objets de départ sont dans le layout, pas sur la case.
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
	UBoardNodeType* GetNodeType() const { return NodeType; }

	// Vrai si le type de la case possède ce comportement (Gameplay Tag, ex. Board.Node.Exit)
	UFUNCTION(BlueprintPure, Category = "Board")
	bool HasBehavior(FGameplayTag Behavior) const;

	// Case infranchissable : comportement Board.Node.Blocked OU blocage temporaire en jeu (SetBlocked)
	UFUNCTION(BlueprintPure, Category = "Board")
	bool IsBlocked() const;

	// Case de sortie : comportement Board.Node.Exit
	UFUNCTION(BlueprintPure, Category = "Board")
	bool IsExit() const;

	// Blocage temporaire en jeu (obstacle, effet d'objet…). Ne modifie pas le niveau.
	void SetBlocked(bool bValue) { bRuntimeBlocked = bValue; }

	// Change le type de la case et met à jour son visuel.
	// Appelé par le Level Editor à la génération ; utilisable en jeu (ex. une case qui change d'état).
	void SetNodeType(UBoardNodeType* InNodeType);

	// Réapplique le visuel du type actuel (ex. après modification du Data Asset du type dans l'éditeur)
	void RefreshVisuals() { ApplyNodeTypeVisuals(); }

#if WITH_EDITORONLY_DATA
	// --- Ancien format, lu uniquement par la migration (commande BoardEditor.ImportLegacy) ---
	TSubclassOf<ABoardPawn> GetLegacyStartingEntity() const { return StartingEntity_DEPRECATED; }
	TSubclassOf<UBoardItem> GetLegacyItem() const { return Item_DEPRECATED; }
	bool IsLegacyExit() const { return bIsExit_DEPRECATED; }
	bool IsLegacyBlocked() const { return bIsBlocked_DEPRECATED; }
#endif

protected:
	// Cale la case sur la grille et applique le visuel de son type.
	// Rejoué au chargement, au déplacement et après un Ctrl+Z dans l'éditeur.
	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	// Après un Ctrl+Z, NodeType a pu changer : on réapplique son visuel
	virtual void PostEditUndo() override;
#endif

	// --- Composants ---

	// Mesh de la case, choisi par son type. Bloque le canal Visibility : c'est lui que le clic du joueur touche.
	UPROPERTY(VisibleAnywhere, Category = "Board")
	TObjectPtr<UStaticMeshComponent> NodeMesh;

	// --- Données (générées par le Level Editor : non éditables) ---

	// Coordonnées sur la grille, calculées depuis la position
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Board")
	FIntPoint GridCoord = FIntPoint::ZeroValue;

	// Type de la case (visuel + comportements)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Board")
	TObjectPtr<UBoardNodeType> NodeType;

	// --- État d'exécution ---

	// Blocage temporaire posé en jeu. Transient : jamais sauvegardé avec le niveau.
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Board")
	bool bRuntimeBlocked = false;

#if WITH_EDITORONLY_DATA
	// --- Ancien format (avant le Level Editor) ---
	// Le suffixe _DEPRECATED permet de relire les niveaux existants sans jamais réécrire ces valeurs.
	// TEMPORAIRE : à supprimer quand tous les niveaux auront été migrés.
	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Remplacé par le layout du ABoard."))
	TSubclassOf<ABoardPawn> StartingEntity_DEPRECATED;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Remplacé par le layout du ABoard."))
	TSubclassOf<UBoardItem> Item_DEPRECATED;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Remplacé par le comportement Board.Node.Exit."))
	bool bIsExit_DEPRECATED = false;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Remplacé par le comportement Board.Node.Blocked."))
	bool bIsBlocked_DEPRECATED = false;
#endif

private:
	// Applique le mesh, l'échelle et les matériaux du type sur NodeMesh
	void ApplyNodeTypeVisuals();

	// Le ABoard du niveau, ou nullptr s'il n'y en a pas
	ABoard* FindBoard() const;
};
