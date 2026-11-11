#include "PlagueTaleGOEditor.h"
#include "PlagueTaleGOEditor/Sync/BoardAutoSync.h"
#include "PlagueTaleGOEditor/Mode/BoardEditorModeCommands.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogBoardEditor);

void FPlagueTaleGOEditorModule::StartupModule()
{
	FBoardAutoSync::Register();
	// Boutons du mode « Plateau » (le mode lui-même est découvert automatiquement par l'éditeur)
	FBoardEditorModeCommands::Register();
	UE_LOG(LogBoardEditor, Log, TEXT("[BoardEditor] Module éditeur chargé."));
}

void FPlagueTaleGOEditorModule::ShutdownModule()
{
	FBoardEditorModeCommands::Unregister();
	FBoardAutoSync::Unregister();
}

IMPLEMENT_MODULE(FPlagueTaleGOEditorModule, PlagueTaleGOEditor)
