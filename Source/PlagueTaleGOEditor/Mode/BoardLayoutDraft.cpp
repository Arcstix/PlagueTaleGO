#include "BoardLayoutDraft.h"
#include "PlagueTaleGO/Board/Board.h"
#include "PlagueTaleGO/Level/BoardPlacementSettings.h"
#include "PlagueTaleGO/Level/Validation/BoardValidator.h"
#include "UObject/UnrealType.h"   // FProperty, TFieldIterator

#define LOCTEXT_NAMESPACE "BoardLayoutDraft"

namespace
{
	// Même contenu : même classe et mêmes valeurs, propriété par propriété.
	// Les objets comparés sont des copies différentes (le brouillon a les siennes) : comparer les pointeurs ne suffit pas.
	bool AreSettingsEqual(const UBoardPlacementSettings* A, const UBoardPlacementSettings* B)
	{
		if (A == B)
		{
			return true;
		}
		if (!A || !B || A->GetClass() != B->GetClass())
		{
			return false;
		}
		for (TFieldIterator<FProperty> It(A->GetClass()); It; ++It)
		{
			for (int32 Index = 0; Index < It->ArrayDim; ++Index)
			{
				// PPF_DeepComparison : compare aussi le contenu des sous-objets instanciés éventuels
				if (!It->Identical_InContainer(A, B, Index, PPF_DeepComparison))
				{
					return false;
				}
			}
		}
		return true;
	}

	// Même type, mêmes réglages, mêmes entités et objets (dans le même ordre)
	bool AreCellsEqual(const FBoardCell& A, const FBoardCell& B)
	{
		if (A.NodeType != B.NodeType || !AreSettingsEqual(A.NodeSettings, B.NodeSettings)
			|| A.Entities.Num() != B.Entities.Num() || A.Items.Num() != B.Items.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < A.Entities.Num(); ++Index)
		{
			const FBoardEntityPlacement& EntityA = A.Entities[Index];
			const FBoardEntityPlacement& EntityB = B.Entities[Index];
			if (EntityA.Type != EntityB.Type || EntityA.Facing != EntityB.Facing || !AreSettingsEqual(EntityA.Settings, EntityB.Settings))
			{
				return false;
			}
		}
		for (int32 Index = 0; Index < A.Items.Num(); ++Index)
		{
			if (A.Items[Index].Type != B.Items[Index].Type || !AreSettingsEqual(A.Items[Index].Settings, B.Items[Index].Settings))
			{
				return false;
			}
		}
		return true;
	}
}

void UBoardLayoutDraft::ResetFromBoard(ABoard* InBoard)
{
	Board = InBoard;
	Layout = InBoard ? InBoard->GetLayout() : FBoardLayout();

	// Les réglages du Board appartiennent au Board : le brouillon travaille sur ses propres copies
	Layout.DeepCopySettings(this);

	bDirty = false;
	Revalidate();
	RefreshModifiedCells();
	OnChanged.Broadcast();
}

void UBoardLayoutDraft::SetLayout(const FBoardLayout& NewLayout)
{
	Layout = NewLayout;

	auto Keep = [this](UBoardPlacementSettings* Settings)
	{
		if (Settings)
		{
			KeepAlive.AddUnique(Settings);
		}
	};
	for (const FBoardCell& Cell : Layout.Cells)
	{
		Keep(Cell.NodeSettings);
		for (const FBoardEntityPlacement& Entity : Cell.Entities)
		{
			Keep(Entity.Settings);
		}
		for (const FBoardItemPlacement& Item : Cell.Items)
		{
			Keep(Item.Settings);
		}
	}

	bDirty = true;
	Revalidate();
	RefreshModifiedCells();
	OnChanged.Broadcast();
}

void UBoardLayoutDraft::CancelTo(const FBoardLayout& PreviousLayout, bool bWasDirty)
{
	SetLayout(PreviousLayout);
	bDirty = bWasDirty;
	OnChanged.Broadcast();   // pour que le panneau affiche le bon état
}

void UBoardLayoutDraft::RefreshModifiedCells()
{
	ModifiedCells.Reset();
	const ABoard* CurrentBoard = Board.Get();
	if (!CurrentBoard)
	{
		return;
	}

	// Quelques dizaines de cases : comparaison complète à chaque changement du brouillon (pas à chaque image)
	const FBoardLayout& Current = CurrentBoard->GetLayout();
	for (const FBoardCell& Cell : Layout.Cells)
	{
		const FBoardCell* Before = Current.FindCell(Cell.Coord);
		bool bModified = !Before || !AreCellsEqual(Cell, *Before);

		// Liens : même nombre et mêmes voisines reliées
		if (!bModified)
		{
			const TArray<FIntPoint> Linked = Layout.GetLinkedCoords(Cell.Coord);
			bModified = Linked.Num() != Current.CountLinks(Cell.Coord);
			for (const FIntPoint& Other : Linked)
			{
				bModified |= !Current.HasLink(Cell.Coord, Other);
			}
		}

		if (bModified)
		{
			ModifiedCells.Add(Cell.Coord);
		}
	}
}

void UBoardLayoutDraft::Revalidate()
{
	Validation = FBoardValidator::Validate(Layout);

	ErrorCells.Reset();
	WarningCells.Reset();
	for (const FBoardValidationMessage& Message : Validation.Messages)
	{
		if (Message.Severity == EBoardValidationSeverity::Error)
		{
			ErrorCells.Append(Message.Cells);
		}
		else
		{
			WarningCells.Append(Message.Cells);
		}
	}
}

FText UBoardLayoutDraft::GetErrorSummary(int32 MaxLines) const
{
	TArray<FString> Lines;
	int32 ErrorCount = 0;
	for (const FBoardValidationMessage& Message : Validation.Messages)
	{
		if (Message.Severity != EBoardValidationSeverity::Error)
		{
			continue;
		}
		++ErrorCount;
		if (Lines.Num() < MaxLines)
		{
			Lines.Add(TEXT("• ") + Message.Text.ToString());
		}
	}

	FString Result = FString::Join(Lines, TEXT("\n"));
	if (ErrorCount > MaxLines)
	{
		Result += TEXT("\n") + FText::Format(LOCTEXT("MoreErrors", "... and {0} more (full list: panel > Draft > Problems)"),
			FText::AsNumber(ErrorCount - MaxLines)).ToString();
	}
	return FText::FromString(Result);
}

#undef LOCTEXT_NAMESPACE
