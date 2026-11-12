#include "PlayerStartRule.h"
#include "PlagueTaleGO/Level/BoardLayout.h"
#include "PlagueTaleGO/Level/Definitions/BoardEntityType.h"

// Textes affichés à l'utilisateur : en anglais, en LOCTEXT (convention UE pour tout FText affiché)
#define LOCTEXT_NAMESPACE "BoardValidation"

UPlayerStartRule::UPlayerStartRule()
{
	Severity = EBoardValidationSeverity::Error;
	Description = LOCTEXT("PlayerStart_Desc", "Exactly one player start in the level.");
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
		Report(OutMessages, LOCTEXT("PlayerStart_None", "No player start: place a player entity on a cell."));
	}
	else if (PlayerCells.Num() > 1)
	{
		Report(OutMessages, FText::Format(LOCTEXT("PlayerStart_Many", "{0} player starts ({1}): exactly one is required."),
			FText::AsNumber(PlayerCells.Num()), FText::FromString(CoordsToString(PlayerCells))), PlayerCells);
	}
}

#undef LOCTEXT_NAMESPACE
