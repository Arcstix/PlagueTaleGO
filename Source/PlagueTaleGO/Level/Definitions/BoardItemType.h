#pragma once

#include "CoreMinimal.h"
#include "BoardDefinition.h"
// Include complet (et non une déclaration anticipée) : tester ou lire ItemClass appelle
// UBoardItem::StaticClass() (vérification de TSubclassOf), qui exige la classe complète.
#include "PlagueTaleGO/Items/BoardItem.h"
#include "BoardItemType.generated.h"

/**
 * Un type d'objet posable sur une case.
 * ⚠️ Le système d'objets n'est pas encore conçu : le Level Editor peut déjà placer des objets
 * (ils sont enregistrés dans le layout), mais rien ne les utilise en jeu pour l'instant.
 */
UCLASS(BlueprintType, meta = (ToolTip = "An item type that the Board Editor can place on cells."))
class PLAGUETALEGO_API UBoardItemType : public UBoardDefinition
{
	GENERATED_BODY()

public:
	// Classe de l'objet (sous-classe de UBoardItem)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ToolTip = "Item class (subclass of UBoardItem)."))
	TSubclassOf<UBoardItem> ItemClass;
};
