// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardNode.h"
#include "Board.h"
#include "BoardTags.h"
#include "EngineUtils.h"   // TActorIterator
#include "Components/StaticMeshComponent.h"
#include "PlagueTaleGO/Entities/ABoardPawn.h"
#include "PlagueTaleGO/Items/BoardItem.h"
#include "PlagueTaleGO/Level/Definitions/BoardNodeType.h"

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
