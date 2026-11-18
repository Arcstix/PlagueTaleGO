// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardNode.h"
#include "Board.h"
#include "BoardTags.h"
#include "EngineUtils.h"   // TActorIterator
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PlagueTaleGO/PlagueTaleGO.h"
#include "PlagueTaleGO/Level/Definitions/BoardNodeType.h"

namespace
{
	// Doivent correspondre exactement aux paramètres du matériau des cases
	const FName HighlightColorParam(TEXT("HighlightColor"));
	const FName HighlightAmountParam(TEXT("HighlightAmount"));
}

ABoardNode::ABoardNode()
{
	// Les nœuds n'ont pas besoin de Tick
	PrimaryActorTick.bCanEverTick = false;

	NodeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NodeMesh"));
	RootComponent = NodeMesh;
}

bool ABoardNode::HasBehavior(FGameplayTag Behavior) const
{
	return NodeType && NodeType->HasBehavior(Behavior);
}

bool ABoardNode::IsBlocked() const
{
	return bRuntimeBlocked || HasBehavior(BoardTags::Node_Blocked);
}

bool ABoardNode::IsExit() const
{
	return HasBehavior(BoardTags::Node_Exit);
}

void ABoardNode::SetNodeType(UBoardNodeType* InNodeType)
{
	NodeType = InNodeType;
	ApplyNodeTypeVisuals();
}

void ABoardNode::SetHighlight(FLinearColor Color, float Amount)
{
	EnsureHighlightMaterials();
	
	const float ClampedAmount = FMath::Clamp(Amount, 0.f, 1.f);
	for (UMaterialInstanceDynamic* Material : HighlightMaterials)
	{
		if (Material)
		{
			Material->SetVectorParameterValue(HighlightColorParam, Color);
			Material->SetScalarParameterValue(HighlightAmountParam, ClampedAmount);
		}
	}
}

void ABoardNode::ClearHighlight()
{
	if (HighlightMaterials.Num() == 0)
	{
		return;   // jamais colorée : inutile de créer des instances pour rien
	}
	SetHighlight(FLinearColor::Black, 0.f);
}

void ABoardNode::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Le visuel est toujours recalculé depuis le type. Modifier l'asset du type met aussi
	// à jour les cases immédiatement (UBoardNodeType::PostEditChangeProperty).
	ApplyNodeTypeVisuals();

	ABoard* Board = FindBoard();
	if (!Board)
	{
		return;   // pas de Board dans le niveau : la case reste où elle est
	}

	// Cale la case sur la grille en X/Y, garde sa hauteur
	GridCoord = Board->WorldToGrid(GetActorLocation());
	const FVector Snapped = Board->GridToWorld(GridCoord);
	SetActorLocation(FVector(Snapped.X, Snapped.Y, GetActorLocation().Z));
}

#if WITH_EDITOR
void ABoardNode::PostEditUndo()
{
	Super::PostEditUndo();
	ApplyNodeTypeVisuals();
}
#endif

void ABoardNode::ApplyNodeTypeVisuals()
{
	if (!NodeMesh)
	{
		return;
	}

	NodeMesh->SetStaticMesh(NodeType ? NodeType->Mesh.Get() : nullptr);
	NodeMesh->SetRelativeScale3D(NodeType ? NodeType->MeshScale : FVector::OneVector);

	// On repart des matériaux du mesh, puis on applique les surcharges du type (case vide = pas de surcharge)
	NodeMesh->EmptyOverrideMaterials();
	HighlightMaterials.Reset();
	if (!NodeType)
	{
		return;
	}
	for (int32 Index = 0; Index < NodeType->MaterialOverrides.Num(); ++Index)
	{
		if (UMaterialInterface* Material = NodeType->MaterialOverrides[Index])
		{
			NodeMesh->SetMaterial(Index, Material);
		}
	}
}

ABoard* ABoardNode::FindBoard() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	TActorIterator<ABoard> It(World);
	return It ? *It : nullptr;
}

void ABoardNode::EnsureHighlightMaterials()
{
	if (!NodeMesh || HighlightMaterials.Num() > 0)
	{
		return;   // déjà créées
	}

	// Une instance dynamique par slot : elle remplace le matériau du slot et permet de changer ses paramètres
	for (int32 Slot = 0; Slot < NodeMesh->GetNumMaterials(); ++Slot)
	{
		if (UMaterialInstanceDynamic* Material = NodeMesh->CreateAndSetMaterialInstanceDynamic(Slot))
		{
			HighlightMaterials.Add(Material);
		}
	}
}
