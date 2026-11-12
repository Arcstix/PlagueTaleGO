#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "BoardDefinition.h"
#include "BoardNodeType.generated.h"

class ABoardNode;
class UStaticMesh;
class UMaterialInterface;

/**
 * Un type de case (ex. « Sol », « Sortie », « Herbes hautes »…).
 * Le visuel ET le comportement d'une case viennent de son type :
 * modifier l'asset met à jour toutes les cases de ce type dans tous les niveaux.
 *
 * - Comportement : Behaviors (Gameplay Tags), testés par le code via ABoardNode::HasBehavior.
 * - Visuel : Mesh / MaterialOverrides / MeshScale, appliqués sur ABoardNode::NodeMesh.
 * - Classe : NodeClass, seulement si ce type a besoin d'un comportement C++ propre.
 */
UCLASS(BlueprintType, meta = (ToolTip = "A cell type for the Board Editor: visuals and behaviors of the cells."))
class PLAGUETALEGO_API UBoardNodeType : public UBoardDefinition
{
	GENERATED_BODY()

public:
	UBoardNodeType();

	UFUNCTION(BlueprintPure, Category = "Node")
	bool HasBehavior(FGameplayTag Tag) const { return Behaviors.HasTag(Tag); }

#if WITH_EDITOR
	// Appelé par l'éditeur après chaque modification de l'asset : met à jour tout de suite
	// le visuel des cases de ce type dans les niveaux ouverts.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Type présélectionné dans le pinceau du Level Editor. Cocher sur UN seul type (en général « Sol »).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Editor", meta = (ToolTip = "Type preselected in the Board Editor brush. Check it on ONE type only (usually Floor)."))
	bool bIsDefaultType = false;

	// --- Comportement ---

	// Comportements de la case (ex. Board.Node.Exit = case de sortie).
	// Seuls les tags testés par le code ont un effet : voir le tableau dans Source/PlagueTaleGOEditor/CLAUDE.md.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node", meta = (Categories = "Board.Node", ToolTip = "Cell behaviors (e.g. Board.Node.Exit = exit cell). Only tags tested by the code have an effect."))
	FGameplayTagContainer Behaviors;

	// Classe d'acteur générée pour cette case. ABoardNode suffit presque toujours :
	// une sous-classe n'est utile que pour un comportement C++ propre à ce type.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node", meta = (ToolTip = "Actor class generated for this cell. BoardNode is almost always enough."))
	TSubclassOf<ABoardNode> NodeClass;

	// --- Visuel ---

	// Mesh de la case. Doit bloquer le canal Visibility pour que le clic du joueur la trouve.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node|Visuals", meta = (ToolTip = "Cell mesh. It needs a collision, otherwise the player's click cannot find the cell."))
	TObjectPtr<UStaticMesh> Mesh;

	// Matériaux remplaçant ceux du mesh (index = slot). Case vide = matériau d'origine du mesh.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node|Visuals", meta = (ToolTip = "Materials replacing the mesh's ones (index = slot). Empty entry = original material."))
	TArray<TObjectPtr<UMaterialInterface>> MaterialOverrides;

	// Échelle appliquée au mesh
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node|Visuals", meta = (ToolTip = "Scale applied to the mesh."))
	FVector MeshScale = FVector::OneVector;
};
