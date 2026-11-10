#include "PlayerStartRule.h"
#include "PlagueTaleGO/Level/BoardLayout.h"
#include "PlagueTaleGO/Level/Definitions/BoardEntityType.h"

UPlayerStartRule::UPlayerStartRule()
{
	Severity = EBoardValidationSeverity::Error;
	Description = INVTEXT("Exactement un départ joueur dans le niveau.");
}

void UPlayerStartRule::Validate(const FBoardLayout& Layout, TArray<FBoardValidationMessage>& OutMessages) const
{
	TArray<FIntPoint> PlayerCells;
	for (const FBoardCell& Cell : Layout.Cells)
	{
		for (const FBoardEntityPlacement& Entity : Cell.Entities)
		{
			if (Entity.Type && Entity.Type->IsPlayer())
			{
				PlayerCells.Add(Cell.Coord);
			}
		}
	}

	if (PlayerCells.Num() == 0)
	{
		Report(OutMessages, INVTEXT("Aucun départ joueur : placer une entité joueur sur une case."));
	}
	else if (PlayerCells.Num() > 1)
	{
		Report(OutMessages, FText::FromString(FString::Printf(
			TEXT("%d départs joueur (%s) : il en faut exactement un."),
			PlayerCells.Num(), *CoordsToString(PlayerCells))), PlayerCells);
	}
}
