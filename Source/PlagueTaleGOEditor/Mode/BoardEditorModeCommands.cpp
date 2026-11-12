#include "BoardEditorModeCommands.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "BoardEditorModeCommands"

FBoardEditorModeCommands::FBoardEditorModeCommands()
	: TCommands<FBoardEditorModeCommands>(
		TEXT("BoardEditorMode"),
		LOCTEXT("ContextDescription", "Board Editor"),
		NAME_None,
		FAppStyle::GetAppStyleSetName())
{
}

void FBoardEditorModeCommands::RegisterCommands()
{
	TArray<TSharedPtr<FUICommandInfo>>& ToolCommands = Commands.FindOrAdd(NAME_Default);

	// UI_COMMAND crée la commande : identifiant, libellé, tooltip, type de bouton, raccourci (aucun)
	UI_COMMAND(EditTool, "Board", "Edit the board: place, paint and erase cells. Hovering a cell shows the available actions.",
		EUserInterfaceActionType::ToggleButton, FInputChord());
	ToolCommands.Add(EditTool);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> FBoardEditorModeCommands::GetCommands()
{
	return Get().Commands;
}

#undef LOCTEXT_NAMESPACE
