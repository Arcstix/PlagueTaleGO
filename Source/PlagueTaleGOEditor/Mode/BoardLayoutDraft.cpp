#include "BoardLayoutDraft.h"
#include "PlagueTaleGO/Board/Board.h"
#include "PlagueTaleGO/Level/BoardPlacementSettings.h"
#include "PlagueTaleGO/Level/Validation/BoardValidator.h"

void UBoardLayoutDraft::ResetFromBoard(ABoard* InBoard)
{
	Board = InBoard;
	Layout = InBoard ? InBoard->GetLayout() : FBoardLayout();

	// Les réglages du Board appartiennent au Board : le brouillon travaille sur ses propres copies
	Layout.DeepCopySettings(this);

	bDirty = false;
	Revalidate();
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
	OnChanged.Broadcast();
}

void UBoardLayoutDraft::CancelTo(const FBoardLayout& PreviousLayout, bool bWasDirty)
{
	SetLayout(PreviousLayout);
	bDirty = bWasDirty;
	OnChanged.Broadcast();   // pour que le panneau affiche le bon état
}

void UBoardLayoutDraft::Revalidate()
{
	Validation = FBoardValidator::Validate(Layout);

	ErrorCells.Reset();
	for (const FBoardValidationMessage& Message : Validation.Messages)
	{
		if (Message.Severity == EBoardValidationSeverity::Error)
		{
			ErrorCells.Append(Message.Cells);
		}
	}
}
