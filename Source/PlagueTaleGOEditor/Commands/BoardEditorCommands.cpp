#include "PlagueTaleGOEditor/PlagueTaleGOEditor.h"
#include "PlagueTaleGOEditor/Sync/BoardLayoutSync.h"
#include "PlagueTaleGO/Board/Board.h"
#include "PlagueTaleGO/Level/BoardLayout.h"
#include "PlagueTaleGO/Level/Validation/BoardValidator.h"
#include "Editor.h"                  // GEditor
#include "ScopedTransaction.h"       // FScopedTransaction (Ctrl+Z)
#include "HAL/IConsoleManager.h"     // FAutoConsoleCommand

/**
 * Commandes console du Level Editor.
 * À taper dans le champ « Cmd » en bas de l'éditeur (ou dans l'Output Log) ; Tab complète le nom.
 * Elles agissent sur le niveau ouvert dans l'éditeur, jamais sur une partie en cours (PIE).
 *
 * FAutoConsoleCommand : un objet global qui enregistre la commande au chargement du module
 * et la retire à son déchargement. Rien d'autre à faire pour qu'elle existe.
 */
namespace
{
	// Le Board du niveau ouvert dans l'éditeur, ou nullptr (avec un message) si la commande ne peut pas s'exécuter
	ABoard* GetEditorBoard()
	{
		if (!GEditor)
		{
			return nullptr;
		}
		if (GEditor->PlayWorld)
		{
			UE_LOG(LogBoardEditor, Warning, TEXT("[BoardEditor] Quitter le Play (PIE) avant d'utiliser cette commande."));
			return nullptr;
		}
		return FBoardLayoutSync::FindBoard(GEditor->GetEditorWorldContext().World());
	}

	void ValidateCommand()
	{
		if (const ABoard* Board = GetEditorBoard())
		{
			FBoardValidator::LogResult(FBoardValidator::Validate(Board->GetLayout()), TEXT("BoardEditor.Validate"));
		}
	}

	void ResyncCommand()
	{
		ABoard* Board = GetEditorBoard();
		if (!Board)
		{
			return;
		}

		const FScopedTransaction Transaction(INVTEXT("Plateau : resynchroniser la scène"));
		FBoardLayoutSync::SyncNodeActors(*Board);
	}

	void ImportLegacyCommand(const TArray<FString>& Args)
	{
		ABoard* Board = GetEditorBoard();
		if (!Board)
		{
			return;
		}

		// Sécurité : ne pas écraser un layout déjà construit avec le Level Editor
		const bool bForce = Args.Contains(FString(TEXT("force")));
		if (!Board->GetLayout().IsEmpty() && !bForce)
		{
			UE_LOG(LogBoardEditor, Warning, TEXT("[BoardEditor] Ce Board a déjà un layout (%d cases) : import annulé. Pour l'écraser : BoardEditor.ImportLegacy force"),
				Board->GetLayout().Cells.Num());
			return;
		}

		TArray<FString> Report;
		const FBoardLayout Layout = FBoardLayoutSync::BuildLayoutFromLegacyScene(*Board, Report);
		for (const FString& Line : Report)
		{
			UE_LOG(LogBoardEditor, Warning, TEXT("[BoardEditor] Import : %s"), *Line);
		}

		FBoardLayoutSync::Submit(*Board, Layout, INVTEXT("Plateau : import de l'ancien format"));

		UE_LOG(LogBoardEditor, Log, TEXT("[BoardEditor] Import terminé : %d case(s), %d lien(s). Sauvegarder le niveau pour le conserver (Ctrl+Z pour annuler)."),
			Layout.Cells.Num(), Layout.Links.Num());

		// À titre d'information : la migration n'est pas bloquée par la validation
		FBoardValidator::LogResult(FBoardValidator::Validate(Board->GetLayout()), TEXT("Import de l'ancien format"));
	}

	FAutoConsoleCommand ValidateCmd(
		TEXT("BoardEditor.Validate"),
		TEXT("Valide le layout du niveau ouvert et affiche le résultat dans l'Output Log."),
		FConsoleCommandDelegate::CreateStatic(&ValidateCommand));

	FAutoConsoleCommand ResyncCmd(
		TEXT("BoardEditor.Resync"),
		TEXT("Régénère les cases de la scène à partir du layout (ex. après avoir changé le NodeClass d'un type de case)."),
		FConsoleCommandDelegate::CreateStatic(&ResyncCommand));

	FAutoConsoleCommand ImportLegacyCmd(
		TEXT("BoardEditor.ImportLegacy"),
		TEXT("Migration : convertit un niveau construit à la main (ancien format) en layout. Argument 'force' pour écraser un layout existant."),
		FConsoleCommandWithArgsDelegate::CreateStatic(&ImportLegacyCommand));
}
