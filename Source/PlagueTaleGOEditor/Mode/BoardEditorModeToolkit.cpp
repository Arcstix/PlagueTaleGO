#include "BoardEditorModeToolkit.h"

#define LOCTEXT_NAMESPACE "BoardEditorModeToolkit"

void FBoardEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	FModeToolkit::Init(InitToolkitHost, InOwningMode);
}

void FBoardEditorModeToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const
{
	// Une seule palette : celle remplie par FBoardEditorModeCommands (clé NAME_Default)
	PaletteNames.Add(NAME_Default);
}

FName FBoardEditorModeToolkit::GetToolkitFName() const
{
	return FName(TEXT("BoardEditorMode"));
}

FText FBoardEditorModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("ToolkitName", "Board");
}

#undef LOCTEXT_NAMESPACE
