// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoardItemPickup.generated.h"

class UStaticMeshComponent;
class UBoardItemType;

/**
 * Visuel d'un objet posé sur une case, créé par le GameMode au lancement de la partie.
 * Purement visuel : aucune logique, aucune collision (le clic du joueur doit atteindre la case en dessous).
 * Le mesh et l'échelle viennent du type d'objet (PickupMesh, PickupScale).
 * Un BP enfant peut ajouter une animation (rotation, flottement) sans toucher au code.
 */
UCLASS()
class PLAGUETALEGO_API ABoardItemPickup : public AActor
{
	GENERATED_BODY()

public:
	ABoardItemPickup();

	// Applique le visuel du type d'objet. Appelé par le GameMode juste après la création.
	void Setup(UBoardItemType* InItemType);

	UFUNCTION(BlueprintPure, Category = "Item")
	UBoardItemType* GetItemType() const { return ItemType; }

protected:
	// --- Composants ---

	UPROPERTY(VisibleAnywhere, Category = "Item")
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	// --- Réglages ---

	// Hauteur du visuel au-dessus du centre de la case (cm)
	UPROPERTY(EditDefaultsOnly, Category = "Item", meta = (ToolTip = "Height of the item above the cell center (cm)."))
	float HeightOffset = 50.f;

	// --- État (debug) ---

	// Type d'objet affiché
	UPROPERTY(VisibleInstanceOnly, Category = "Item")
	TObjectPtr<UBoardItemType> ItemType;
};
