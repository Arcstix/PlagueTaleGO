#include "PlagueTaleGOEditor.h"
#include "PlagueTaleGOEditor/Sync/BoardAutoSync.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogBoardEditor);

void FPlagueTaleGOEditorModule::StartupModule()
{
	FBoardAutoSync::Register();
	UE_LOG(LogBoardEditor, Log, TEXT("[BoardEditor] Module éditeur chargé."));
}

void FPlagueTaleGOEditorModule::ShutdownModule()
{
	FBoardAutoSync::Unregister();
}

IMPLEMENT_MODULE(FPlagueTaleGOEditorModule, PlagueTaleGOEditor)
