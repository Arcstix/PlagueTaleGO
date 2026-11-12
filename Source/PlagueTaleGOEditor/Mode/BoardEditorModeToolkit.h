#pragma once

#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"

/**
 * Panneau du mode « Board Editor » (à gauche de l'éditeur quand le mode est actif).
 *
 * On garde le panneau standard d'UE : barre d'outils générée depuis FBoardEditorModeCommands,
 * puis les propriétés de l'outil actif (UBoardEditToolProperties), affichées comme un panneau Details.
 * Les boutons « Submit » / « Discard Draft » sont des UFUNCTION(CallInEditor) de ces propriétés :
 * aucun code Slate n'est nécessaire.
 */
class FBoardEditorModeToolkit : public FModeToolkit
{
public:
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode) override;
	virtual void GetToolPaletteNames(TArray<FName>& PaletteNames) const override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
};
