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
	PawnToNode.Reset();

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
	SetPawnNode(Entity, Node);
}

void UBoardSubsystem::UnregisterPawn(ABoardPawn* Pawn)
{
	if (!Pawn)
	{
		return;
	}
	// nullptr = libère la case occupée et retire le pion de PawnToNode
	SetPawnNode(Pawn, nullptr);
}

ABoardNode* UBoardSubsystem::GetPawnNode(const ABoardPawn* Pawn) const
{
	ABoardNode* Found = PawnToNode.FindRef(Pawn);
	return Found ? Found : nullptr;
}

bool UBoardSubsystem::IsOccupied(const ABoardNode* Node) const
{
	const FNodeOccupants* Found = Occupancy.Find(Node);
	return Found && Found->Entities.Num() > 0;
}

ABoardPawn* UBoardSubsystem::GetPawnOn(const ABoardNode* Node) const
{
	const FNodeOccupants* Found = Occupancy.Find(Node);
	if (!Found || Found->Entities.Num() == 0)
	{
		return nullptr;
	}
	// Une seule entité par case pour l'instant : on renvoie la première
	return Found->Entities[0];
}

bool UBoardSubsystem::IsExit(const ABoardNode* Node) const
{
	return Node && Node->IsExit();
}

TArray<ABoardNode*> UBoardSubsystem::FindPath(ABoardNode* From, ABoardNode* To, bool bAvoidOccupied) const
{
	TArray<ABoardNode*> Path;
	if (!From || !To || From == To || To->IsBlocked())
	{
		return Path;
	}

	// File d'attente du parcours (on avance avec un index plutôt que de retirer en tête)
	TArray<ABoardNode*> Queue = { From };

	// Pour chaque case visitée : la case d'où l'on vient (sert à reconstruire le chemin)
	TMap<ABoardNode*, ABoardNode*> CameFrom;
	CameFrom.Add(From, nullptr);

	bool bFound = false;

	for (int32 Index = 0; Index < Queue.Num() && !bFound; ++Index)
	{
		ABoardNode* Current = Queue[Index];

		// GetNeighbors applique déjà la règle des 4 directions et des liens bloqués
		for (ABoardNode* Next : GetNeighbors(Current))
		{
			if (CameFrom.Contains(Next))
			{
				continue;   // déjà visitée
			}
			if (Next != To)
			{
				if (Next->IsBlocked())
				{
					continue;
				}
				if (bAvoidOccupied && IsOccupied(Next))
				{
					continue;
				}
			}

			CameFrom.Add(Next, Current);

			if (Next == To)
			{
				bFound = true;
				break;
			}
			Queue.Add(Next);
		}
	}

	if (!bFound)
	{
		return Path;   // aucun chemin
	}

	// Remonte de l'arrivée vers le départ, puis remet dans l'ordre
	for (ABoardNode* Step = To; Step && Step != From; Step = CameFrom[Step])
	{
		Path.Add(Step);
	}
	Algo::Reverse(Path);
	return Path;
}

void UBoardSubsystem::SetPawnNode(ABoardPawn* Pawn, ABoardNode* NewNode)
{
	// Libère l'ancienne case
	if (ABoardNode* OldNode = GetPawnNode(Pawn))
	{
		if (FNodeOccupants* OldOccupants = Occupancy.Find(OldNode))
		{
			OldOccupants->Entities.Remove(Pawn);
		}
	}

	// Occupe la nouvelle (nullptr = le pion quitte le plateau)
	if (NewNode)
	{
		PawnToNode.Add(Pawn, NewNode);
		Occupancy.FindOrAdd(NewNode).Entities.AddUnique(Pawn);
	}
	else
	{
		PawnToNode.Remove(Pawn);
	}
}

EMoveResult UBoardSubsystem::TryMovePawn(ABoardPawn* Pawn, ABoardNode* InTargetNode)
{
	if (!Pawn || !InTargetNode)
	{
		return EMoveResult::Blocked;
	}
	if (Pawn->IsMoving())
	{
		return EMoveResult::AlreadyMoving;
	}
	
	ABoardNode* FromNode = GetPawnNode(Pawn);

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

	SetPawnNode(Pawn, InTargetNode);
	Pawn->BeginMove(InTargetNode);
	return EMoveResult::Accepted;
}
