// PlayerPawn.cpp
#include "APlayerPawn.h"
#include "../../Node/BoardSubsystem.h"
#include "Developer/AITestSuite/Public/AITestsCommon.h"
#include "Engine/World.h"

EMoveResult APlayerPawn::RequestMove(AANode* InTargetNode)
{
	UBoardSubsystem* Board = FAITestHelpers::GetWorld()->GetSubsystem<UBoardSubsystem>();
	if (!Board)
	{
		return EMoveResult::Blocked;
	}

	return Board->TryMovePawn(this, InTargetNode);
}