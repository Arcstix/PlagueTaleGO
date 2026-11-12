#include "IsolatedNodeRule.h"
#include "PlagueTaleGO/Level/BoardLayout.h"

// Textes affichés à l'utilisateur : en anglais, en LOCTEXT (convention UE pour tout FText affiché)
#define LOCTEXT_NAMESPACE "BoardValidation"

UIsolatedNodeRule::UIsolatedNodeRule()
{
	Severity = EBoardValidationSeverity::Error;
	Description = LOCTEXT("IsolatedNode_Desc", "Every cell has at least one link.");
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
		Report(OutMessages, FText::Format(LOCTEXT("IsolatedNode_Message", "{0} cell(s) without any link: {1}. Link these cells or remove them."),
			FText::AsNumber(Isolated.Num()), FText::FromString(CoordsToString(Isolated))), Isolated);
	}
}

#undef LOCTEXT_NAMESPACE
