#pragma once

#include "CoreMinimal.h"
#include "InteractiveTool.h"
#include "InteractiveToolBuilder.h"
#include "BaseBehaviors/BehaviorTargetInterfaces.h"
#include "InteractiveToolQueryInterfaces.h"   // IInteractiveToolNestedAcceptCancelAPI (Échap)
#include "PlagueTaleGO/Level/BoardLayout.h"
#include "BoardEditTool.generated.h"

class UBoardEditorMode;
class UBoardEditTool;
class UBoardLayoutDraft;
class UBoardNodeType;
class ABoard;

/**
 * Crée l'outil « Plateau » quand le mode le demande.
 * Mécanisme ITF : le gestionnaire d'outils ne crée jamais un outil directement, il passe par son « builder ».
 */
UCLASS()
class UBoardEditToolBuilder : public UInteractiveToolBuilder
{
	GENERATED_BODY()

public:
	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;

	// Le mode propriétaire (donne accès au brouillon)
	TWeakObjectPtr<UBoardEditorMode> Mode;
};

/**
 * Panneau de l'outil « Plateau » : affiché automatiquement par le mode, comme un panneau Details.
 * Les UFUNCTION(CallInEditor) deviennent des boutons. Les commentaires des UPROPERTY sont les tooltips.
 */
UCLASS()
class UBoardEditToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	// --- Pinceau ---

	// Type des cases posées ou tracées, et appliqué par Ctrl + clic. Présélectionné : le type coché « Type par défaut ».
	UPROPERTY(EditAnywhere, Category = "Pinceau", meta = (DisplayName = "Type de case"))
	TObjectPtr<UBoardNodeType> PaintType;

	// --- Brouillon ---

	// Développement uniquement : autorise le Submit même si la validation trouve des erreurs.
	UPROPERTY(EditAnywhere, Category = "Brouillon", meta = (DisplayName = "Ignorer les erreurs (dev)"))
	bool bIgnoreErrors = false;

	// Résumé du brouillon : modifié ou non, nombre de cases et de liens, erreurs
	UPROPERTY(VisibleAnywhere, Category = "Brouillon", meta = (DisplayName = "État"))
	FString Status;

	// Problèmes trouvés par la validation. Les erreurs bloquent le Submit, les avertissements informent.
	UPROPERTY(VisibleAnywhere, Category = "Brouillon", meta = (DisplayName = "Problèmes"))
	TArray<FString> Problems;

	// Valide le brouillon puis l'écrit dans le niveau (un seul Ctrl+Z pour annuler)
	UFUNCTION(CallInEditor, Category = "Brouillon", meta = (DisplayName = "Soumettre"))
	void Submit();

	// Abandonne toutes les modifications non soumises
	UFUNCTION(CallInEditor, Category = "Brouillon", meta = (DisplayName = "Annuler le brouillon"))
	void Revert();

	// --- Vue ---

	// Replace la caméra à la verticale du plateau, cadrée sur toutes les cases
	UFUNCTION(CallInEditor, Category = "Vue", meta = (DisplayName = "Vue de dessus"))
	void FrameBoard();

	// --- Aide ---

	// Liste des gestes de l'outil
	UPROPERTY(VisibleAnywhere, Category = "Aide", meta = (DisplayName = "Commandes", MultiLine = true))
	FText HelpText;

	// L'outil qui possède ce panneau (pour les boutons)
	TWeakObjectPtr<UBoardEditTool> ParentTool;
};

/**
 * L'outil « Plateau » : tous les gestes du Level Editor dans le viewport.
 * Spécification : CLAUDE.md, § 1 « Gestes de l'outil Plateau ».
 *
 * Bouton gauche (UClickDragInputBehavior) : un appui démarre un « tracé » dont la nature dépend des touches :
 * - sans touche : tracer un chemin (crée les cases manquantes et relie chaque case à la précédente) ;
 *   un clic sans bouger sur une case vide la pose, sur une case existante ouvre ses détails (étape 3) ;
 * - Ctrl : appliquer le type du pinceau aux cases survolées ;
 * - Shift : un clic sans bouger efface le lien visé (ou la case) ; en glissant, efface les cases survolées.
 * Le tracé modifie le brouillon en direct (aperçu), puis forme UNE étape d'historique au relâchement.
 * Échap l'annule : l'éditeur propose d'abord Échap à l'outil (IInteractiveToolNestedAcceptCancelAPI) ;
 * hors tracé, Échap arrête l'outil et le mode le relance (les réglages du panneau sont conservés).
 *
 * Le clic droit n'est pas utilisé : il reste réservé à la caméra (décision d'équipe, éviter la confusion).
 */
UCLASS()
class UBoardEditTool : public UInteractiveTool, public IClickDragBehaviorTarget, public IHoverBehaviorTarget,
	public IInteractiveToolNestedAcceptCancelAPI
{
	GENERATED_BODY()

public:
	// Appelé par le builder, avant Setup
	void Init(UBoardEditorMode* InMode);

	// --- UInteractiveTool ---
	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	virtual void Render(IToolsContextRenderAPI* RenderAPI) override;
	virtual void OnPropertyModified(UObject* PropertySet, FProperty* Property) override;

	// --- IClickDragBehaviorTarget : bouton gauche (clic ou glisser) ---
	virtual FInputRayHit CanBeginClickDragSequence(const FInputDeviceRay& PressPos) override;
	virtual void OnClickPress(const FInputDeviceRay& PressPos) override;
	virtual void OnClickDrag(const FInputDeviceRay& DragPos) override;
	virtual void OnClickRelease(const FInputDeviceRay& ReleasePos) override;
	virtual void OnTerminateDragSequence() override;

	// --- IHoverBehaviorTarget : survol ---
	virtual FInputRayHit BeginHoverSequenceHitTest(const FInputDeviceRay& PressPos) override;
	virtual void OnBeginHover(const FInputDeviceRay& DevicePos) override;
	virtual bool OnUpdateHover(const FInputDeviceRay& DevicePos) override;
	virtual void OnEndHover() override;

	// --- Touches de modification (Shift, Ctrl) ---
	virtual void OnUpdateModifierState(int ModifierID, bool bIsOn) override;

	// --- IInteractiveToolNestedAcceptCancelAPI : Échap pendant un tracé ---
	virtual bool SupportsNestedCancelCommand() override { return true; }
	virtual bool CanCurrentlyNestedCancel() override { return IsStrokeActive(); }
	virtual bool ExecuteNestedCancelCommand() override;

	// --- Appelé par le mode ---

	bool IsStrokeActive() const { return StrokeKind != EStrokeKind::None && !bStrokeCancelled; }

	// Échap : annule le tracé en cours (le brouillon revient à son état d'avant le tracé)
	void CancelStroke();

	// --- Boutons du panneau ---
	void RequestSubmit();
	void RequestRevert();
	void RequestFrameBoard();

private:
	// Nature du tracé en cours (bouton gauche maintenu)
	enum class EStrokeKind : uint8
	{
		None,
		Paint,       // tracer un chemin relié
		ApplyType,   // Ctrl : appliquer le type du pinceau
		Erase        // Shift : effacer
	};

	UBoardLayoutDraft* GetDraft() const;
	ABoard* GetBoard() const;

	// Point du plan du Board touché par un rayon. Faux si le rayon ne le touche pas devant la caméra.
	bool RayToPoint(const FVector& Origin, const FVector& Direction, FVector& OutPoint, double& OutDepth) const;

	// Case de la grille sous la souris
	bool RayToCell(const FInputDeviceRay& Ray, FIntPoint& OutCell, double& OutDepth) const;

	// Que vise ce point ? Toujours une case ; un lien du brouillon si le point est entre deux cases reliées.
	void HitTest(const FVector& Point, FIntPoint& OutCell, TOptional<FBoardLink>& OutLink) const;

	/**
	 * Modification ponctuelle du brouillon (hors tracé ; servira aux détails d'une case, étape 3) :
	 * Edit reçoit une copie du layout et renvoie true s'il l'a changée.
	 * Le brouillon est mis à jour et l'avant/après ajouté à l'historique Ctrl+Z.
	 */
	void ApplyDraftEdit(const FText& Description, TFunctionRef<bool(FBoardLayout&)> Edit);

	// --- Tracé ---

	// Action du tracé sur une case visitée. Renvoie true si StrokeLayout a changé.
	bool StrokeVisit(FIntPoint Cell);

	// Tracé de chemin : de From à To (voisines) — crée To si besoin et relie les deux. Renvoie true si changé.
	bool PaintStep(FIntPoint From, FIntPoint To);

	// Envoie StrokeLayout au brouillon (aperçu en direct, sans historique)
	void PushStrokeToDraft();

	void ResetStroke();

	// --- Affichage ---

	// Le brouillon a changé : met à jour État / Problèmes dans le panneau
	void HandleDraftChanged();

	// Affiche l'aide contextuelle correspondant à ce qui est survolé
	void UpdateHoverMessage();

	UPROPERTY()
	TObjectPtr<UBoardEditToolProperties> Properties;

	TWeakObjectPtr<UBoardEditorMode> Mode;

	// Survol : case sous la souris, et lien visé s'il y en a un
	TOptional<FIntPoint> HoveredCell;
	TOptional<FBoardLink> HoveredLink;

	bool bShiftDown = false;
	bool bCtrlDown = false;

	// --- État du tracé en cours ---
	EStrokeKind StrokeKind = EStrokeKind::None;
	FBoardLayout StrokeStartLayout;   // brouillon avant le tracé (pour l'historique et Échap)
	FBoardLayout StrokeLayout;        // brouillon pendant le tracé
	bool bStrokeWasDirty = false;     // état « modifié » du brouillon avant le tracé
	bool bStrokeChanged = false;      // le tracé a modifié quelque chose
	bool bStrokeMoved = false;        // la souris a quitté la case de départ
	bool bStrokeCancelled = false;    // Échap pendant le tracé
	bool bStartCellExisted = false;   // la case de départ existait avant le tracé
	FIntPoint LastStrokeCell = FIntPoint::ZeroValue;
	TOptional<FBoardLink> StrokeStartLink;   // Shift : lien visé à l'appui (effacé si on relâche sans bouger)

	FDelegateHandle DraftChangedHandle;

	// Identifiants des touches de modification auprès des comportements d'entrée
	static constexpr int32 ShiftModifierId = 1;
	static constexpr int32 CtrlModifierId = 2;
};
