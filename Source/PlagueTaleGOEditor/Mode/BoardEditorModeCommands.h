#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

/**
 * Commandes (boutons) du mode « Plateau » : elles forment la barre d'outils du mode.
 * Une seule pour l'instant : l'outil « Plateau », qui regroupe tous les gestes (voir CLAUDE.md, § Interactions).
 *
 * Mécanisme UE : TCommands déclare des actions d'interface (libellé, tooltip, raccourci éventuel).
 * Elles sont enregistrées au démarrage du module (FPlagueTaleGOEditorModule::StartupModule).
 */
class FBoardEditorModeCommands : public TCommands<FBoardEditorModeCommands>
{
public:
	FBoardEditorModeCommands();

	virtual void RegisterCommands() override;

	// Commandes rangées par palette (nom de palette → boutons), lues par UBoardEditorMode::GetModeCommands
	static TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> GetCommands();

	// Outil unique « Plateau »
	TSharedPtr<FUICommandInfo> EditTool;

private:
	TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> Commands;
};
