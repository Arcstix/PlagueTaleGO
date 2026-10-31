// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardNode.h"
#include "Board.h"
#include "EngineUtils.h"   // TActorIterator

// Sets default values
ABoardNode::ABoardNode()
{
	// Les nœuds n'ont pas besoin de Tick
	PrimaryActorTick.bCanEverTick = false;
}

void ABoardNode::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

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
void ABoardNode::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	// Seulement quand on lâche la case, pour ne pas tout redessiner à chaque pixel
	if (bFinished)
	{
		if (ABoard* Board = FindBoard())
		{
			Board->RebuildLinkVisuals();
		}
	}
}
#endif

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
