#include "IsolatedNodeRule.h"
#include "PlagueTaleGO/Level/BoardLayout.h"

UIsolatedNodeRule::UIsolatedNodeRule()
{
	Severity = EBoardValidationSeverity::Error;
	Description = INVTEXT("Chaque case a au moins un lien.");
}

void UIsolatedNodeRule::Validate(const FBoardLayout& Layout, TArray<FBoardValidationMessage>& OutMessages) const
{
	TArray<FIntPoint> Isolated;
	for (const FBoardCell& Cell : Layout.Cells)
	{
		if (Layout.CountLinks(Cell.Coord) == 0)
		{
			Isolated.Add(Cell.Coord);
		}
	}

	if (Isolated.Num() > 0)
	{
		Report(OutMessages, FText::FromString(FString::Printf(
			TEXT("%d case(s) sans aucun lien : %s. Relier ces cases ou les supprimer."),
			Isolated.Num(), *CoordsToString(Isolated))), Isolated);
	}
}
