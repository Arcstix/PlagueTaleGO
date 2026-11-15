#pragma once

#include "CoreMinimal.h"
#include "BoardDefinition.h"
// Include complet (et non une déclaration anticipée) : tester ou lire ItemClass appelle
// UBoardItem::StaticClass() (vérification de TSubclassOf), qui exige la classe complète.
#include "PlagueTaleGO/Items/BoardItem.h"
#include "BoardItemType.generated.h"

class UStaticMesh;

/**
 * Un type d'objet posable sur une case (ex. « Artefact de lumière »).
 * Le Level Editor place des objets de ce type sur les cases ; en jeu, le GameMode affiche
 * leur visuel (PickupMesh) et le joueur prend l'objet (ItemClass) en arrivant sur la case.
 * Le nombre de fois qu'une case donne l'objet est une règle du type (choix du GD) :
 * une fois, X fois (MaxUses) ou à chaque passage (bUnlimitedUses).
 */
UCLASS(BlueprintType, meta = (ToolTip = "An item type that the Board Editor can place on cells."))
class PLAGUETALEGO_API UBoardItemType : public UBoardDefinition
{
	GENERATED_BODY()

public:
	// Classe de l'objet (sous-classe de UBoardItem)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ToolTip = "Item class (subclass of UBoardItem)."))
	TSubclassOf<UBoardItem> ItemClass;

	// --- Utilisations ---

	// Vrai : la case donne l'objet à chaque fois que le joueur y arrive, sans jamais s'épuiser
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Uses", meta = (
		DisplayName = "Unlimited Uses",
		ToolTip = "If checked, the cell gives the item every time the player steps on it and never runs out."))
	bool bUnlimitedUses = false;

	// Nombre de fois que la case donne l'objet avant de redevenir une case normale.
	// Ignoré (et grisé dans l'éditeur) si bUnlimitedUses est coché.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Uses", meta = (
		ClampMin = "1",
		EditCondition = "!bUnlimitedUses",
		DisplayName = "Max Uses",
		ToolTip = "How many times the cell gives the item before it becomes a normal cell."))
	int32 MaxUses = 1;

	// --- Visuel ---

	// Mesh affiché sur la case tant qu'elle donne l'objet. Sans collision en jeu : le clic doit atteindre la case.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Visuals", meta = (
		ToolTip = "Mesh shown on the cell while it still gives the item."))
	TObjectPtr<UStaticMesh> PickupMesh;

	// Échelle appliquée au mesh
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Visuals", meta = (
		ToolTip = "Scale applied to the pickup mesh."))
	FVector PickupScale = FVector::OneVector;
};
