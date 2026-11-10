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
UCLASS(BlueprintType)
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

	// --- Comportement ---

	// Comportements de la case (ex. Board.Node.Exit = case de sortie).
	// Seuls les tags testés par le code ont un effet : voir le tableau dans Source/PlagueTaleGOEditor/CLAUDE.md.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node", meta = (Categories = "Board.Node"))
	FGameplayTagContainer Behaviors;

	// Classe d'acteur générée pour cette case. ABoardNode suffit presque toujours :
	// une sous-classe n'est utile que pour un comportement C++ propre à ce type.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node")
	TSubclassOf<ABoardNode> NodeClass;

	// --- Visuel ---

	// Mesh de la case. Doit bloquer le canal Visibility pour que le clic du joueur la trouve.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node|Visuals")
	TObjectPtr<UStaticMesh> Mesh;

	// Matériaux remplaçant ceux du mesh (index = slot). Case vide = matériau d'origine du mesh.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node|Visuals")
	TArray<TObjectPtr<UMaterialInterface>> MaterialOverrides;

	// Échelle appliquée au mesh
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node|Visuals")
	FVector MeshScale = FVector::OneVector;
};
