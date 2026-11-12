#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

// Catégorie de log du module éditeur. Tag des messages : [BoardEditor].
// (LogPlagueTaleGO n'est pas exportée par le module runtime : on ne peut pas l'utiliser ici.)
DECLARE_LOG_CATEGORY_EXTERN(LogBoardEditor, Log, All);

/**
 * Module éditeur du projet.
 * - Au démarrage : active la resynchronisation automatique de la scène (FBoardAutoSync)
 *   et enregistre les boutons du mode « Board Editor » (FBoardEditorModeCommands).
 * - Les commandes console s'enregistrent toutes seules (FAutoConsoleCommand),
 *   et le mode éditeur (UBoardEditorMode, un UEdMode) est découvert automatiquement par l'éditeur.
 */
class FPlagueTaleGOEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
