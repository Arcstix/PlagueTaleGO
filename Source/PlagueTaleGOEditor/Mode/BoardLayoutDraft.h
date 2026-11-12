#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PlagueTaleGO/Level/BoardLayout.h"
#include "PlagueTaleGO/Level/Validation/BoardValidationTypes.h"
#include "BoardLayoutDraft.generated.h"

class ABoard;
class UBoardPlacementSettings;

/**
 * Brouillon du mode « Board Editor » : une copie du layout du Board, modifiée par l'outil.
 * Tant qu'il n'est pas soumis, ni le Board ni la scène ne changent : seul l'aperçu montre le brouillon.
 *
 * - Créé par UBoardEditorMode à l'entrée dans le mode, soumis par UBoardEditorMode::SubmitDraft.
 * - Chaque modification remplace le layout entier (SetLayout) : c'est simple, et les niveaux sont petits.
 *   L'outil enregistre l'avant/après dans l'historique Ctrl+Z (FBoardDraftChange).
 * - Le brouillon se revalide après chaque modification : l'aperçu et le panneau affichent les problèmes.
 */
UCLASS()
class UBoardLayoutDraft : public UObject
{
	GENERATED_BODY()

public:
	// (Re)part du layout actuel du Board. Après cet appel, le brouillon n'est plus « modifié ».
	void ResetFromBoard(ABoard* InBoard);

	// Remplace le layout du brouillon. Utilisé par les modifications de l'outil et par Ctrl+Z / Ctrl+Y.
	void SetLayout(const FBoardLayout& NewLayout);

	// Revient à un layout précédent en restaurant aussi l'état « modifié » (ex. Échap pendant un tracé)
	void CancelTo(const FBoardLayout& PreviousLayout, bool bWasDirty);

	ABoard* GetBoard() const { return Board.Get(); }
	const FBoardLayout& GetLayout() const { return Layout; }

	// Vrai si le brouillon a été modifié depuis le dernier Submit / la dernière annulation
	bool IsDirty() const { return bDirty; }

	const FBoardValidationResult& GetValidation() const { return Validation; }

	// Cases citées par au moins une erreur / un avertissement (cercle rouge / orange dans l'aperçu)
	const TSet<FIntPoint>& GetErrorCells() const { return ErrorCells; }
	const TSet<FIntPoint>& GetWarningCells() const { return WarningCells; }

	// Cases du brouillon qui diffèrent du niveau (nouvelle case, type, entités, objets, réglages ou liens) :
	// carré fin dans l'aperçu, jusqu'au prochain Submit. Les cases supprimées n'y sont pas (croix grise).
	const TSet<FIntPoint>& GetModifiedCells() const { return ModifiedCells; }

	// Nombre d'erreurs listées dans les fenêtres de message (Submit refusé, sortie du mode)
	static constexpr int32 DialogMaxErrors = 5;

	// Les erreurs en liste à puces (au plus MaxLines, puis « ... and N more ») pour une fenêtre de message
	FText GetErrorSummary(int32 MaxLines = DialogMaxErrors) const;

	// Diffusé après chaque changement du brouillon (l'outil met à jour son panneau)
	FSimpleMulticastDelegate OnChanged;

private:
	void Revalidate();

	// Recalcule ModifiedCells en comparant le brouillon au layout du Board
	void RefreshModifiedCells();

	// Le Board dont on édite le layout (référence faible : le niveau peut être fermé)
	TWeakObjectPtr<ABoard> Board;

	UPROPERTY()
	FBoardLayout Layout;

	// L'historique Ctrl+Z garde des copies de layouts qui pointent vers des objets de réglages.
	// Cet ensemble les garde en vie (sinon le garbage collector pourrait les détruire).
	// TSet : ajout sans recherche linéaire, à chaque modification du brouillon.
	UPROPERTY()
	TSet<TObjectPtr<UBoardPlacementSettings>> KeepAlive;

	bool bDirty = false;

	FBoardValidationResult Validation;
	TSet<FIntPoint> ErrorCells;
	TSet<FIntPoint> WarningCells;
	TSet<FIntPoint> ModifiedCells;
};
