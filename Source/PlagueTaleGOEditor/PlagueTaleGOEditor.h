#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

// Catégorie de log du module éditeur. Tag des messages : [BoardEditor].
// (LogPlagueTaleGO n'est pas exportée par le module runtime : on ne peut pas l'utiliser ici.)
DECLARE_LOG_CATEGORY_EXTERN(LogBoardEditor, Log, All);

/**
 * Module éditeur du projet.
 * - Au démarrage : active la resynchronisation automatique de la scène (FBoardAutoSync)
 *   et enregistre les boutons du mode « Board » (FBoardEditorModeCommands).
 * - Les commandes console s'enregistrent toutes seules (FAutoConsoleCommand),
 *   et le futur mode éditeur (UEdMode) sera découvert automatiquement par l'éditeur.
 */
class FPlagueTaleGOEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
