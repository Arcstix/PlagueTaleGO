#pragma once

#include "CoreMinimal.h"
#include "BoardTypes.generated.h"

UENUM(BlueprintType)
enum class EBoardPawnState : uint8
{
	Idle,
	Moving
};

// Résultat renvoyé par le Board à chaque demande de déplacement
UENUM(BlueprintType)
enum class EMoveResult : uint8
{
	Accepted,
	NotAdjacent,
	Blocked,
	Occupied,
	AlreadyMoving
};