#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Level/BoardPlacementSettings.h"
#include "PlayerPlacementSettings.generated.h"

/**
 * Réglages du placement du joueur dans un niveau (Board Editor > case du joueur > Selected Cell > entité joueur).
 * Activé en choisissant cette classe dans DA_Entity_Player > Settings Class.
 * Reçu par APlayerPawn::ApplyPlacementSettings, avant BeginPlay.
 *
 * Chaque réglage a une case « Override » : décochée, la valeur du BP du joueur s'applique.
 * Ainsi, changer la valeur par défaut dans BP_PlayerPawn met à jour tous les niveaux qui ne la remplacent pas.
 */
UCLASS(meta = (DisplayName = "Player Placement Settings"))
class PLAGUETALEGO_API UPlayerPlacementSettings : public UBoardPlacementSettings
{
	GENERATED_BODY()

public:
	// Coché : TrailLength remplace la durée de la trace de poussière bleue réglée dans BP_PlayerPawn, pour ce niveau
	UPROPERTY(EditAnywhere, Category = "BlueDust", meta = (InlineEditConditionToggle,
		ToolTip = "Override the blue dust trail length of BP_PlayerPawn for this level."))
	bool bOverrideTrailLength = false;

	// Nombre de déplacements pendant lesquels une trace reste dans ce niveau (2 = N-1 et N-2). 0 = aucune trace.
	UPROPERTY(EditAnywhere, Category = "BlueDust", meta = (EditCondition = "bOverrideTrailLength", ClampMin = "0", UIMax = "10",
		DisplayName = "Blue Dust Trail Length",
		ToolTip = "Number of player moves a blue dust trace stays on its cell in this level (2 = the last two cells left). 0 = no trace."))
	int32 TrailLength = 2;
};
