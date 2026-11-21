#include "ExitRule.h"
#include "PlagueTaleGO/Level/BoardLayout.h"
#include "PlagueTaleGO/Board/BoardTags.h"
#include "PlagueTaleGO/Level/Definitions/BoardNodeType.h"

// Textes affichés à l'utilisateur : en anglais, en LOCTEXT (convention UE pour tout FText affiché)
#define LOCTEXT_NAMESPACE "BoardValidation"

UExitRule::UExitRule()
{
	Severity = EBoardValidationSeverity::Error;
	Description = LOCTEXT("Exit_Desc", "At least one exit cell (Board.Node.Exit behavior).");
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

	Report(OutMessages, LOCTEXT("Exit_None", "No exit cell: use a cell type with the Board.Node.Exit behavior."));
}

#undef LOCTEXT_NAMESPACE
