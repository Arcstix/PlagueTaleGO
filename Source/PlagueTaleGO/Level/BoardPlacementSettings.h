#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BoardPlacementSettings.generated.h"

/**
 * Réglages d'UN élément placé dans le niveau (une case, une entité ou un objet),
 * propres à son type. Exemple futur : un ennemi qui patrouille aurait une sous-classe
 * avec sa direction de patrouille.
 *
 * Fonctionnement :
 * - un Data Asset de définition (UBoardDefinition) choisit sa classe de réglages dans SettingsClass ;
 * - le Level Editor crée UNE instance de cette classe par placement et l'affiche dans son panneau,
 *   avec les tooltips de chaque UPROPERTY ;
 * - au lancement de la partie, le pion la reçoit dans ABoardPawn::ApplyPlacementSettings.
 *
 * Ajouter des réglages à un type = créer une sous-classe C++ (UPROPERTY(EditAnywhere) avec un ToolTip en anglais),
 * puis la sélectionner dans le Data Asset. Le Level Editor n'a pas besoin d'être modifié.
 *
 * Mécanismes UE :
 * - EditInlineNew : l'objet peut être créé directement dans un panneau Details ;
 * - DefaultToInstanced : chaque référence possède SA copie (« sous-objet instancié »), dupliquée
 *   avec son propriétaire. Deux placements ne partagent jamais le même objet de réglages.
 * Équivalent Godot : une sous-ressource « Local to Scene » propre à chaque nœud.
 */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced, CollapseCategories)
class PLAGUETALEGO_API UBoardPlacementSettings : public UObject
{
	GENERATED_BODY()
};
