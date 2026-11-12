#include "IntegrityRule.h"
#include "PlagueTaleGO/Level/BoardLayout.h"

// Textes affichés à l'utilisateur : en anglais, en LOCTEXT (convention UE pour tout FText affiché)
#define LOCTEXT_NAMESPACE "BoardValidation"

UIntegrityRule::UIntegrityRule()
{
	Severity = EBoardValidationSeverity::Error;
	Description = LOCTEXT("Integrity_Desc", "Consistent layout data: unique coordinates, links between existing neighboring cells.");
}

void UIntegrityRule::Validate(const FBoardLayout& Layout, TArray<FBoardValidationMessage>& OutMessages) const
{
	// 1. Coordonnées en double
	TSet<FIntPoint> Seen;
	TArray<FIntPoint> Duplicates;
	for (const FBoardCell& Cell : Layout.Cells)
	{
		if (Seen.Contains(Cell.Coord))
		{
			Duplicates.AddUnique(Cell.Coord);
		}
		Seen.Add(Cell.Coord);
	}
	if (Duplicates.Num() > 0)
	{
		Report(OutMessages, FText::Format(LOCTEXT("Integrity_Duplicates", "Several cells share the same coordinates: {0}."),
			FText::FromString(CoordsToString(Duplicates))), Duplicates);
	}

	// 2. Liens invalides
	for (const FBoardLink& Link : Layout.Links)
	{
		if (!Layout.HasCell(Link.A) || !Layout.HasCell(Link.B))
		{
			Report(OutMessages, FText::Format(LOCTEXT("Integrity_MissingCell", "The link {0} - {1} connects a cell that does not exist."),
				FText::FromString(CoordToString(Link.A)), FText::FromString(CoordToString(Link.B))), { Link.A, Link.B });
		}
		else if (Link.A == Link.B || !FBoardLink::AreAdjacent(Link.A, Link.B))
		{
			Report(OutMessages, FText::Format(LOCTEXT("Integrity_NotAdjacent", "The link {0} - {1} does not connect two neighboring cells (4 directions, no diagonal)."),
				FText::FromString(CoordToString(Link.A)), FText::FromString(CoordToString(Link.B))), { Link.A, Link.B });
		}
	}
}

#undef LOCTEXT_NAMESPACE
