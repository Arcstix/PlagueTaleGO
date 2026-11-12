#include "ConnectedGraphRule.h"
#include "PlagueTaleGO/Level/BoardLayout.h"

// Textes affichés à l'utilisateur : en anglais, en LOCTEXT (convention UE pour tout FText affiché)
#define LOCTEXT_NAMESPACE "BoardValidation"

UConnectedGraphRule::UConnectedGraphRule()
{
	Severity = EBoardValidationSeverity::Error;
	Description = LOCTEXT("ConnectedGraph_Desc", "All cells are connected to each other (no isolated island).");
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

	Report(OutMessages, FText::Format(LOCTEXT("ConnectedGraph_Message", "The board is split into several parts: {0} cell(s) are not connected to the rest ({1})."),
		FText::AsNumber(Unreachable.Num()), FText::FromString(CoordsToString(Unreachable))), Unreachable);
}

#undef LOCTEXT_NAMESPACE
