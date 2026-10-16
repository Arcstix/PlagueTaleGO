// PlayerPawn.cpp
#include "APlayerPawn.h"
#include "../../Node/BoardSubsystem.h"
#include "Engine/World.h"

EMoveResult APlayerPawn::RequestMove(AANode* InTargetNode)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return EMoveResult::Blocked;
	}

	UBoardSubsystem* Board = World->GetSubsystem<UBoardSubsystem>();
	if (!Board)
	{
		return EMoveResult::Blocked;
	}

	return Board->TryMovePawn(this, InTargetNode);
}
