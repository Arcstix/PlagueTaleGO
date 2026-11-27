// PlayerPawn.cpp
#include "APlayerPawn.h"
#include "PlagueTaleGO/Board/BoardSubsystem.h"
#include "PlagueTaleGO/BlueDust/BlueDustTrailComponent.h"
#include "Engine/World.h"

APlayerPawn::APlayerPawn()
{
	// Composant « par défaut » (CreateDefaultSubobject) : existe dans la classe et dans tous ses BP enfants
	BlueDustTrail = CreateDefaultSubobject<UBlueDustTrailComponent>(TEXT("BlueDustTrail"));
}


