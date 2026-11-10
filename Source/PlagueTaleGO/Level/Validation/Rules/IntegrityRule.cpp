#include "IntegrityRule.h"
#include "PlagueTaleGO/Level/BoardLayout.h"

UIntegrityRule::UIntegrityRule()
{
	Severity = EBoardValidationSeverity::Error;
	Description = INVTEXT("Données du layout cohérentes : coordonnées uniques, liens entre cases voisines existantes.");
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
		Report(OutMessages, FText::FromString(FString::Printf(
			TEXT("Plusieurs cases partagent les mêmes coordonnées : %s."), *CoordsToString(Duplicates))), Duplicates);
	}

	// 2. Liens invalides
	for (const FBoardLink& Link : Layout.Links)
	{
		if (!Layout.HasCell(Link.A) || !Layout.HasCell(Link.B))
		{
			Report(OutMessages, FText::FromString(FString::Printf(
				TEXT("Le lien %s - %s relie une case qui n'existe pas."),
				*CoordToString(Link.A), *CoordToString(Link.B))), { Link.A, Link.B });
		}
		else if (Link.A == Link.B || !FBoardLink::AreAdjacent(Link.A, Link.B))
		{
			Report(OutMessages, FText::FromString(FString::Printf(
				TEXT("Le lien %s - %s ne relie pas deux cases voisines (4 directions, pas de diagonale)."),
				*CoordToString(Link.A), *CoordToString(Link.B))), { Link.A, Link.B });
		}
	}
}
