#include "MissingDefinitionRule.h"
#include "PlagueTaleGO/Level/BoardLayout.h"
#include "PlagueTaleGO/Level/Definitions/BoardNodeType.h"
#include "PlagueTaleGO/Level/Definitions/BoardEntityType.h"
#include "PlagueTaleGO/Level/Definitions/BoardItemType.h"

UMissingDefinitionRule::UMissingDefinitionRule()
{
	Severity = EBoardValidationSeverity::Error;
	Description = INVTEXT("Chaque case, entité et objet a un type valide (Data Asset existant et complet).");
}

void UMissingDefinitionRule::Validate(const FBoardLayout& Layout, TArray<FBoardValidationMessage>& OutMessages) const
{
	for (const FBoardCell& Cell : Layout.Cells)
	{
		const FString Where = CoordToString(Cell.Coord);

		if (!Cell.NodeType)
		{
			Report(OutMessages, FText::FromString(FString::Printf(
				TEXT("La case %s n'a pas de type de case."), *Where)), { Cell.Coord });
		}

		for (const FBoardEntityPlacement& Entity : Cell.Entities)
		{
			if (!Entity.Type)
			{
				Report(OutMessages, FText::FromString(FString::Printf(
					TEXT("Une entité de la case %s n'a pas de type (Data Asset supprimé ?)."), *Where)), { Cell.Coord });
			}
			else if (!Entity.Type->PawnClass)
			{
				Report(OutMessages, FText::FromString(FString::Printf(
					TEXT("Le type d'entité %s (case %s) n'a pas de PawnClass."),
					*Entity.Type->GetName(), *Where)), { Cell.Coord });
			}
		}

		for (const FBoardItemPlacement& Item : Cell.Items)
		{
			if (!Item.Type)
			{
				Report(OutMessages, FText::FromString(FString::Printf(
					TEXT("Un objet de la case %s n'a pas de type (Data Asset supprimé ?)."), *Where)), { Cell.Coord });
			}
			else if (!Item.Type->ItemClass)
			{
				Report(OutMessages, FText::FromString(FString::Printf(
					TEXT("Le type d'objet %s (case %s) n'a pas d'ItemClass."),
					*Item.Type->GetName(), *Where)), { Cell.Coord });
			}
		}
	}
}
