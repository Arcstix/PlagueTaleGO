#include "ConnectedGraphRule.h"
#include "PlagueTaleGO/Level/BoardLayout.h"

UConnectedGraphRule::UConnectedGraphRule()
{
	Severity = EBoardValidationSeverity::Error;
	Description = INVTEXT("Toutes les cases sont reliées entre elles (pas d'îlot isolé).");
}

void UConnectedGraphRule::Validate(const FBoardLayout& Layout, TArray<FBoardValidationMessage>& OutMessages) const
{
	if (Layout.Cells.Num() < 2)
	{
		return;
	}

	// Tout ce qui n'est pas atteignable depuis la première case forme un ou plusieurs îlots
	const TSet<FIntPoint> Reachable = Layout.GetReachableCoords(Layout.Cells[0].Coord);
	if (Reachable.Num() >= Layout.Cells.Num())
	{
		return;
	}

	TArray<FIntPoint> Unreachable;
	for (const FBoardCell& Cell : Layout.Cells)
	{
		if (!Reachable.Contains(Cell.Coord))
		{
			Unreachable.Add(Cell.Coord);
		}
	}

	Report(OutMessages, FText::FromString(FString::Printf(
		TEXT("Le plateau est coupé en plusieurs morceaux : %d case(s) ne sont pas reliées au reste (%s)."),
		Unreachable.Num(), *CoordsToString(Unreachable))), Unreachable);
}
