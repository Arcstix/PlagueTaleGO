// BoardSubsystem.cpp

#include "BoardSubsystem.h"
#include "Board.h"
#include "BoardNode.h"
#include "../Entities/ABoardPawn.h"
#include "PlagueTaleGO/PlagueTaleGO.h"
#include "EngineUtils.h"

namespace
{
	// Les 4 directions de la grille : est, ouest, nord, sud
	const FIntPoint GridDirections[] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
}

void UBoardSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	NodesByCoord.Reset();
	Occupancy.Reset();

	// 1. Le Board du niveau (grille + liens bloqués)
	TActorIterator<ABoard> BoardIt(&InWorld);
	Board = BoardIt ? *BoardIt : nullptr;
	if (!Board)
	{
		UE_LOG(LogPlagueTaleGO, Error, TEXT("[Board] Aucun ABoard dans le niveau : graphe vide."));
		return;
	}

	// 2. Toutes les cases, rangées par coordonnées
	for (TActorIterator<ABoardNode> It(&InWorld); It; ++It)
	{
		ABoardNode* Node = *It;
		const FIntPoint Coord = Node->GetGridCoord();

		if (const TObjectPtr<ABoardNode>* Existing = NodesByCoord.Find(Coord))
		{
			UE_LOG(LogPlagueTaleGO, Error, TEXT("[Board] %s et %s partagent la case (%d, %d) : %s ignorée."),
				*GetNameSafe(*Existing), *GetNameSafe(Node), Coord.X, Coord.Y, *GetNameSafe(Node));
			continue;
		}
		NodesByCoord.Add(Coord, Node);
	}

	UE_LOG(LogPlagueTaleGO, Log, TEXT("[Board] Graphe construit : %d cases, %d liens bloqués."),
		NodesByCoord.Num(), Board->BlockedLinks.Num());
}

ABoardNode* UBoardSubsystem::GetNodeAt(FIntPoint Coord) const
{
	const TObjectPtr<ABoardNode>* Found = NodesByCoord.Find(Coord);
	return Found ? Found->Get() : nullptr;
}

bool UBoardSubsystem::AreLinked(const ABoardNode* A, const ABoardNode* B) const
{
	if (!A || !B || A == B || !Board)
	{
		return false;
	}

	const FIntPoint CoordA = A->GetGridCoord();
	const FIntPoint CoordB = B->GetGridCoord();

	// Voisines = exactement 1 case d'écart, sur X ou sur Y (pas en diagonale)
	const FIntPoint Delta = CoordB - CoordA;
	if (FMath::Abs(Delta.X) + FMath::Abs(Delta.Y) != 1)
	{
		return false;
	}

	// Les deux cases doivent être celles enregistrées (pas une case en doublon ignorée)
	if (GetNodeAt(CoordA) != A || GetNodeAt(CoordB) != B)
	{
		return false;
	}

	return !Board->IsLinkBlocked(CoordA, CoordB);
}

TArray<ABoardNode*> UBoardSubsystem::GetNeighbors(const ABoardNode* Node) const
{
	TArray<ABoardNode*> Result;
	if (!Node || !Board)
	{
		return Result;
	}

	const FIntPoint Coord = Node->GetGridCoord();
	for (const FIntPoint& Dir : GridDirections)
	{
		ABoardNode* Other = GetNodeAt(Coord + Dir);
		if (Other && !Board->IsLinkBlocked(Coord, Coord + Dir))
		{
			Result.Add(Other);
		}
	}
	return Result;
}

void UBoardSubsystem::RegisterPawn(ABoardPawn* Entity, ABoardNode* Node)
{
	if (!Entity || !Node)
	{
		return;
	}
	Occupancy.FindOrAdd(Node).Entities.AddUnique(Entity);
}

bool UBoardSubsystem::IsOccupied(const ABoardNode* Node) const
{
	const FNodeOccupants* Found = Occupancy.Find(Node);
	return Found && Found->Entities.Num() > 0;
}

EMoveResult UBoardSubsystem::TryMovePawn(ABoardPawn* Entity, ABoardNode* InTargetNode)
{
	if (!Entity || !InTargetNode)
	{
		return EMoveResult::Blocked;
	}
	if (Entity->IsMoving())
	{
		return EMoveResult::AlreadyMoving;
	}
	
	ABoardNode* FromNode = Entity->GetCurrentNode();

	if (!FromNode)
	{
		return EMoveResult::NoOrigin;
	}
	if (!AreLinked(FromNode, InTargetNode))
	{
		return EMoveResult::NotAdjacent;
	}
	if (InTargetNode->IsBlocked())
	{
		return EMoveResult::Blocked;
	}
	if (IsOccupied(InTargetNode))
	{
		return EMoveResult::Occupied;
	}

	// Libère l'ancienne case, occupe la nouvelle
	if (FNodeOccupants* FromOccupants = Occupancy.Find(FromNode))
	{
		FromOccupants->Entities.Remove(Entity);
	}
	Occupancy.FindOrAdd(InTargetNode).Entities.AddUnique(Entity);

	Entity->BeginMove(InTargetNode);
	return EMoveResult::Accepted;
}
