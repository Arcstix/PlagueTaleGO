// BoardSubsystem.cpp
#include "BoardSubsystem.h"
#include "../Entities/ABoardPawn.h"
#include "BoardNode.h"

void UBoardSubsystem::RegisterPawn(ABoardPawn* Entity, ABoardNode* Node)
{
	if (!Entity || !Node)
	{
		return;
	}
	Occupancy.FindOrAdd(Node).Entities.AddUnique(Entity);
}

bool UBoardSubsystem::IsOccupied(ABoardNode* Node)
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
