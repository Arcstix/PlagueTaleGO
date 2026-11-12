#include "MissingDefinitionRule.h"
#include "PlagueTaleGO/Level/BoardLayout.h"
#include "PlagueTaleGO/Level/Definitions/BoardNodeType.h"
#include "PlagueTaleGO/Level/Definitions/BoardEntityType.h"
#include "PlagueTaleGO/Level/Definitions/BoardItemType.h"

// Textes affichés à l'utilisateur : en anglais, en LOCTEXT (convention UE pour tout FText affiché)
#define LOCTEXT_NAMESPACE "BoardValidation"

UMissingDefinitionRule::UMissingDefinitionRule()
{
	Severity = EBoardValidationSeverity::Error;
	Description = LOCTEXT("MissingDefinition_Desc", "Every cell, entity and item has a valid type (existing and complete Data Asset).");
}

void UMissingDefinitionRule::Validate(const FBoardLayout& Layout, TArray<FBoardValidationMessage>& OutMessages) const
{
	for (const FBoardCell& Cell : Layout.Cells)
	{
		const FText Where = FText::FromString(CoordToString(Cell.Coord));

		if (!Cell.NodeType)
		{
			Report(OutMessages, FText::Format(LOCTEXT("MissingDefinition_NodeType", "The cell {0} has no cell type."), Where), { Cell.Coord });
		}

		for (const FBoardEntityPlacement& Entity : Cell.Entities)
		{
			if (!Entity.Type)
			{
				Report(OutMessages, FText::Format(LOCTEXT("MissingDefinition_EntityType", "An entity on the cell {0} has no type (deleted Data Asset?)."), Where), { Cell.Coord });
			}
			else if (!Entity.Type->PawnClass)
			{
				Report(OutMessages, FText::Format(LOCTEXT("MissingDefinition_PawnClass", "The entity type {0} (cell {1}) has no Pawn Class."),
					FText::FromString(Entity.Type->GetName()), Where), { Cell.Coord });
			}
		}

		for (const FBoardItemPlacement& Item : Cell.Items)
		{
			if (!Item.Type)
			{
				Report(OutMessages, FText::Format(LOCTEXT("MissingDefinition_ItemType", "An item on the cell {0} has no type (deleted Data Asset?)."), Where), { Cell.Coord });
			}
			else if (!Item.Type->ItemClass)
			{
				Report(OutMessages, FText::Format(LOCTEXT("MissingDefinition_ItemClass", "The item type {0} (cell {1}) has no Item Class."),
					FText::FromString(Item.Type->GetName()), Where), { Cell.Coord });
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
