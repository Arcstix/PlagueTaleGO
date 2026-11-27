// PlayerPawn.cpp
#include "APlayerPawn.h"
#include "PlagueTaleGO/Board/BoardSubsystem.h"
#include "PlagueTaleGO/BlueDust/BlueDustTrailComponent.h"
#include "PlagueTaleGO/PlagueTaleGO.h"
#include "PlayerPlacementSettings.h"
#include "Engine/World.h"

APlayerPawn::APlayerPawn()
{
	// Composant « par défaut » (CreateDefaultSubobject) : existe dans la classe et dans tous ses BP enfants
	BlueDustTrail = CreateDefaultSubobject<UBlueDustTrailComponent>(TEXT("BlueDustTrail"));
}

void APlayerPawn::ApplyPlacementSettings(const UBoardPlacementSettings* Settings)
{
	Super::ApplyPlacementSettings(Settings);

	// Pas de réglages : DA_Entity_Player sans Settings Class, ou placement soumis avant son ajout
	// (il les reçoit à sa prochaine édition dans le Board Editor). Les valeurs du BP s'appliquent.
	const UPlayerPlacementSettings* PlayerSettings = Cast<UPlayerPlacementSettings>(Settings);
	if (!PlayerSettings || !BlueDustTrail)
	{
		return;
	}

	if (PlayerSettings->bOverrideTrailLength)
	{
		BlueDustTrail->SetTrailLength(PlayerSettings->TrailLength);
		UE_LOG(LogPlagueTaleGO, Log, TEXT("[BlueDust] Durée de la trace remplacée par le niveau : %d"), PlayerSettings->TrailLength);
	}
}


