#include "BoardEntityType.h"
#include "PlagueTaleGO/Entities/Player/APlayerPawn.h"

bool UBoardEntityType::IsPlayer() const
{
	return PawnClass && PawnClass->IsChildOf(APlayerPawn::StaticClass());
}
