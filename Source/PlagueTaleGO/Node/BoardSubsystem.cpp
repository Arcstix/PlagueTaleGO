// BoardSubsystem.cpp
#include "BoardSubsystem.h"
#include "../Pawn/ABoardPawn.h"
#include "ANode.h"

void UBoardSubsystem::RegisterPawn(ABoardPawn* Entity, AANode* Node)
{
	if (!Entity || !Node)
	{
		return;
	}
	Occupancy.FindOrAdd(Node).Entities.AddUnique(Entity);
}

bool UBoardSubsystem::IsOccupied(AANode* Node)
{
	const FNodeOccupants* Found = Occupancy.Find(Node);
	return Found && Found->Entities.Num() > 0;
}

EMoveResult UBoardSubsystem::TryMovePawn(ABoardPawn* Entity, AANode* InTargetNode)
{
	if (!Entity || !InTargetNode)
	{
		return EMoveResult::Blocked;
	}
	if (Entity->IsMoving())
	{
		return EMoveResult::AlreadyMoving;
	}
	
	AANode* FromNode = Entity->GetCurrentNode();

	if (!FromNode)
	{
		return EMoveResult::NoOrigin;
	}
	if (!FromNode->IsConnectedTo(InTargetNode))
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
