#include "ExitRule.h"
#include "PlagueTaleGO/Level/BoardLayout.h"
#include "PlagueTaleGO/Board/BoardTags.h"
#include "PlagueTaleGO/Level/Definitions/BoardNodeType.h"

UExitRule::UExitRule()
{
	Severity = EBoardValidationSeverity::Error;
	Description = INVTEXT("Au moins une case de sortie (comportement Board.Node.Exit).");
}

void UExitRule::Validate(const FBoardLayout& Layout, TArray<FBoardValidationMessage>& OutMessages) const
{
	for (const FBoardCell& Cell : Layout.Cells)
	{
		if (Cell.NodeType && Cell.NodeType->HasBehavior(BoardTags::Node_Exit))
		{
			return;   // au moins une sortie : la règle est respectée
		}
	}

	Report(OutMessages, INVTEXT("Aucune case de sortie : utiliser un type de case avec le comportement Board.Node.Exit."));
}
