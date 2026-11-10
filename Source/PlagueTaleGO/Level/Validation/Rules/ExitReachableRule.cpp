#include "ExitReachableRule.h"
#include "PlagueTaleGO/Level/BoardLayout.h"
#include "PlagueTaleGO/Board/BoardTags.h"
#include "PlagueTaleGO/Level/Definitions/BoardNodeType.h"
#include "PlagueTaleGO/Level/Definitions/BoardEntityType.h"

UExitReachableRule::UExitReachableRule()
{
	Severity = EBoardValidationSeverity::Warning;
	Description = INVTEXT("Une sortie est atteignable depuis le départ joueur (sans tenir compte des ennemis).");
}

void UExitReachableRule::Validate(const FBoardLayout& Layout, TArray<FBoardValidationMessage>& OutMessages) const
{
	// 1. Départ joueur (exactement un, sinon UPlayerStartRule s'en charge)
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
	if (PlayerCells.Num() != 1)
	{
		return;
	}

	// 2. Parcours depuis le départ en évitant les cases bloquées
	auto IsExit = [](const FBoardCell& Cell)
	{
		return Cell.NodeType && Cell.NodeType->HasBehavior(BoardTags::Node_Exit);
	};
	auto CanEnter = [](const FBoardCell& Cell)
	{
		return !(Cell.NodeType && Cell.NodeType->HasBehavior(BoardTags::Node_Blocked));
	};

	bool bHasExit = false;
	for (const FBoardCell& Cell : Layout.Cells)
	{
		bHasExit |= IsExit(Cell);
	}
	if (!bHasExit)
	{
		return;   // UExitRule s'en charge
	}

	for (const FIntPoint& Coord : Layout.GetReachableCoords(PlayerCells[0], CanEnter))
	{
		const FBoardCell* Cell = Layout.FindCell(Coord);
		if (Cell && IsExit(*Cell))
		{
			return;   // une sortie est atteignable
		}
	}

	Report(OutMessages, FText::FromString(FString::Printf(
		TEXT("Aucune sortie n'est atteignable depuis le départ joueur %s (liens manquants ou cases bloquées)."),
		*CoordToString(PlayerCells[0]))), PlayerCells);
}
