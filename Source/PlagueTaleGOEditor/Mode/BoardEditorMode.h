#pragma once

#include "CoreMinimal.h"
#include "Tools/UEdMode.h"
#include "Editor.h"   // FEditorDelegates, FCanLoadMap
#include "BoardEditorMode.generated.h"

class UBoardLayoutDraft;
class UBoardEditTool;
class FBoardEditorInputProcessor;
class FLevelEditorViewportClient;
class UInteractiveToolManager;
class UInteractiveTool;

/**
 * Le mode éditeur « Board » : le Level Editor du projet.
 * On l'ouvre depuis le menu des modes de l'éditeur (comme Landscape ou Foliage).
 *
 * Responsabilités :
 * - à l'entrée : copier le layout du Board dans un brouillon (UBoardLayoutDraft) et lancer l'outil « Board » ;
 * - soumettre (valider puis écrire dans le Board + régénérer la scène) ou abandonner le brouillon ;
 * - à la sortie (ou au changement de niveau) : demander quoi faire d'un brouillon non soumis ;
 * - caméra : vue de dessus à l'entrée, retour à la vue d'avant à la sortie ;
 * - garder l'outil « Board » actif : s'il s'arrête (Échap hors tracé, bouton de la barre d'outils),
 *   il est relancé au tick suivant.
 * Le clic droit n'est pas utilisé par le mode : il reste réservé à la caméra.
 *
 * Les gestes (clic, glisser…) sont dans l'outil : UBoardEditTool.
 *
 * Mécanisme UE : toute sous-classe de UEdMode est découverte automatiquement par l'éditeur
 * (rien à enregistrer). Équivalent Godot : un EditorPlugin qui prend la main sur le viewport.
 */
UCLASS()
class UBoardEditorMode : public UEdMode
{
	GENERATED_BODY()

public:
	// Identifiant unique du mode
	static const FEditorModeID ModeId;

	// Identifiant de l'outil « Board » auprès du gestionnaire d'outils
	static const FString EditToolName;

	UBoardEditorMode();

	// --- UEdMode ---
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void CreateToolkit() override;
	virtual TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> GetModeCommands() const override;

	virtual void ModeTick(float DeltaTime) override;

	// Appelé par le gestionnaire d'outils quand un outil s'arrête
	virtual void OnToolEnded(UInteractiveToolManager* Manager, UInteractiveTool* Tool) override;

	// --- Caméra ---

	// Place la caméra du viewport à la verticale du plateau, cadrée sur toutes les cases du brouillon
	void FrameBoardFromTop();

	// Déplace la caméra (sans changer son orientation ni sa distance) pour centrer cette case à l'écran
	void FocusCell(FIntPoint Coord);

	// Échap (reçu par FBoardEditorInputProcessor) : true si l'outil s'en est servi
	bool HandleEscape();

	// --- Brouillon ---
	UBoardLayoutDraft* GetDraft() const { return Draft; }

	/**
	 * Valide le brouillon puis l'écrit dans le Board et régénère la scène (un seul Ctrl+Z).
	 * Refusé (renvoie false) s'il contient des erreurs, sauf si bIgnoreErrors est vrai.
	 */
	bool SubmitDraft(bool bIgnoreErrors);

	// Abandonne les modifications : le brouillon repart du layout actuel du Board
	void RevertDraft();

private:
	// Appelé juste avant l'ouverture d'un autre niveau (l'ancien existe encore)
	void HandleMapLoad(const FString& Filename, FCanLoadMap& OutCanLoadMap);

	// Si le brouillon a des modifications non soumises : demande Soumettre / Abandonner
	void AskAboutPendingDraft();

	// L'outil « Board » s'il est actif
	UBoardEditTool* GetEditTool() const;

	// Démarre l'outil « Board » (le seul outil du mode)
	void StartEditTool();

	// Viewport perspective à piloter : celui en cours d'utilisation, sinon le premier viewport perspective
	static FLevelEditorViewportClient* FindPerspectiveViewport();

	void SaveCamera();
	void RestoreCamera();

	UPROPERTY()
	TObjectPtr<UBoardLayoutDraft> Draft;

	FDelegateHandle MapLoadHandle;

	// Reçoit Échap avant l'éditeur (voir FBoardEditorInputProcessor)
	TSharedPtr<FBoardEditorInputProcessor> InputProcessor;

	// L'outil s'est arrêté pendant que le mode est actif : le relancer au prochain tick
	bool bRestartToolPending = false;

	// Vrai pendant Exit : ne pas relancer l'outil qu'on est en train d'arrêter
	bool bIsExiting = false;

	// --- Caméra d'avant l'entrée dans le mode ---
	FLevelEditorViewportClient* SavedViewportClient = nullptr;
	FVector SavedViewLocation = FVector::ZeroVector;
	FRotator SavedViewRotation = FRotator::ZeroRotator;
};
