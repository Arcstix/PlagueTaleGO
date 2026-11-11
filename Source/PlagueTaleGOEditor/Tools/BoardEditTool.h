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
class UBoardPlacementSettings;
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
 * Panneau de l'outil « Plateau » (1/3) : pinceau, brouillon, vue.
 * Les propriétés d'outil sont affichées automatiquement par le mode, comme un panneau Details.
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

	// Description du type choisi (texte d'aide du Data Asset)
	UPROPERTY(VisibleAnywhere, Category = "Pinceau", meta = (DisplayName = "Description", MultiLine = true))
	FText PaintTypeDescription;

	// Couleur du type choisi dans l'aperçu
	UPROPERTY(VisibleAnywhere, Category = "Pinceau", meta = (DisplayName = "Couleur dans l'aperçu"))
	FLinearColor PaintTypeColor = FLinearColor::White;

	// --- Brouillon ---

	// Développement uniquement : autorise le Submit même si la validation trouve des erreurs.
	UPROPERTY(EditAnywhere, Category = "Brouillon", meta = (DisplayName = "Ignorer les erreurs (dev)"))
	bool bIgnoreErrors = false;

	// Résumé du brouillon : modifié ou non, nombre de cases et de liens, erreurs
	UPROPERTY(VisibleAnywhere, Category = "Brouillon", meta = (DisplayName = "État"))
	FString Status;

	// Problèmes trouvés par la validation. Les erreurs bloquent le Submit, les avertissements informent.
	// EditFixedSize : pas de boutons + / poubelle (liste en lecture seule).
	UPROPERTY(VisibleAnywhere, Category = "Brouillon", meta = (DisplayName = "Problèmes", EditFixedSize))
	TArray<FString> Problems;

	// Valide le brouillon puis l'écrit dans le niveau (un seul Ctrl+Z pour annuler)
	UFUNCTION(CallInEditor, Category = "Brouillon", meta = (DisplayName = "Soumettre"))
	void Submit();

	// Abandonne toutes les modifications non soumises
	UFUNCTION(CallInEditor, Category = "Brouillon", meta = (DisplayName = "Annuler le brouillon"))
	void Revert();

	// Sélectionne la prochaine case concernée par un problème et centre la caméra dessus
	UFUNCTION(CallInEditor, Category = "Brouillon", meta = (DisplayName = "Erreur suivante"))
	void NextError();

	// --- Vue ---

	// Replace la caméra à la verticale du plateau, cadrée sur toutes les cases
	UFUNCTION(CallInEditor, Category = "Vue", meta = (DisplayName = "Vue de dessus"))
	void FrameBoard();

	// L'outil qui possède ce panneau (pour les boutons)
	TWeakObjectPtr<UBoardEditTool> ParentTool;
};

/**
 * Panneau de l'outil « Plateau » (2/3) : la case sélectionnée (clic simple sur une case).
 * Chaque modification est appliquée au brouillon (avec Ctrl+Z). Les réglages se créent tout seuls
 * selon le type choisi (SettingsClass du Data Asset).
 *
 * Mécanisme UE : le panneau du mode ne liste les propriétés de l'outil qu'au démarrage de l'outil ;
 * on ne peut donc pas ajouter / retirer cette section à la volée. Elle est toujours présente, et ses lignes
 * sont cachées sans sélection grâce à EditCondition + EditConditionHides (réévalués en continu par le panneau).
 */
UCLASS()
class UBoardCellDetailsProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	// Vrai quand une case est sélectionnée : pilote l'affichage des lignes ci-dessous (non affiché)
	UPROPERTY(Transient)
	bool bHasSelection = false;

	// Affiché seulement quand aucune case n'est sélectionnée
	UPROPERTY(VisibleAnywhere, Transient, Category = "Case sélectionnée", meta = (DisplayName = "Sélection", EditCondition = "!bHasSelection", EditConditionHides))
	FText NoSelectionHint;

	// Position de la case sur la grille
	UPROPERTY(VisibleAnywhere, Category = "Case sélectionnée", meta = (DisplayName = "Coordonnées", EditCondition = "bHasSelection", EditConditionHides))
	FIntPoint Coord = FIntPoint::ZeroValue;

	// Type de la case : son visuel et ses comportements
	UPROPERTY(EditAnywhere, Category = "Case sélectionnée", meta = (DisplayName = "Type de case", EditCondition = "bHasSelection", EditConditionHides))
	TObjectPtr<UBoardNodeType> NodeType;

	// Description du type (texte d'aide du Data Asset)
	UPROPERTY(VisibleAnywhere, Category = "Case sélectionnée", meta = (DisplayName = "Description du type", MultiLine = true, EditCondition = "bHasSelection", EditConditionHides))
	FText TypeDescription;

	// Réglages propres à cette case (seulement si le type en déclare : SettingsClass)
	UPROPERTY(EditAnywhere, Instanced, Category = "Case sélectionnée", meta = (DisplayName = "Réglages de la case", EditCondition = "bHasSelection", EditConditionHides))
	TObjectPtr<UBoardPlacementSettings> NodeSettings;

	// Entités créées sur cette case au lancement (départ joueur, ennemis…). « + » pour en ajouter.
	UPROPERTY(EditAnywhere, Category = "Case sélectionnée", meta = (DisplayName = "Entités", EditCondition = "bHasSelection", EditConditionHides))
	TArray<FBoardEntityPlacement> Entities;

	// Objets posés sur cette case. « + » pour en ajouter.
	UPROPERTY(EditAnywhere, Category = "Case sélectionnée", meta = (DisplayName = "Objets", EditCondition = "bHasSelection", EditConditionHides))
	TArray<FBoardItemPlacement> Items;

	// Problèmes de validation qui concernent cette case
	UPROPERTY(VisibleAnywhere, Category = "Case sélectionnée", meta = (DisplayName = "Problèmes de la case", EditFixedSize, EditCondition = "bHasSelection", EditConditionHides))
	TArray<FString> CellProblems;

	// Ferme cette section (Échap fait de même)
	UFUNCTION(CallInEditor, Category = "Case sélectionnée", meta = (DisplayName = "Désélectionner"))
	void Deselect();

	TWeakObjectPtr<UBoardEditTool> ParentTool;
};

/**
 * Panneau de l'outil « Plateau » (3/3) : l'aide, toujours en bas.
 */
UCLASS()
class UBoardEditToolHelpProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	// Liste des gestes de l'outil
	UPROPERTY(VisibleAnywhere, Category = "Aide", meta = (DisplayName = "Commandes", MultiLine = true))
	FText HelpText;
};

/**
 * L'outil « Plateau » : tous les gestes du Level Editor dans le viewport.
 * Spécification : CLAUDE.md, § 1 « Gestes de l'outil Plateau ».
 *
 * Bouton gauche (UClickDragInputBehavior) : un appui démarre un « tracé » dont la nature dépend des touches :
 * - sans touche : tracer un chemin (crée les cases manquantes et relie chaque case à la précédente) ;
 *   un clic sans bouger sur une case vide la pose, sur une case existante la sélectionne (détails) ;
 * - Ctrl : appliquer le type du pinceau aux cases survolées ;
 * - Shift : un clic sans bouger efface le lien visé (ou la case) ; en glissant, efface les cases survolées.
 * Le tracé modifie le brouillon en direct (aperçu), puis forme UNE étape d'historique au relâchement.
 *
 * Échap : l'éditeur le propose d'abord à l'outil (IInteractiveToolNestedAcceptCancelAPI) : il annule le tracé
 * en cours, sinon désélectionne la case. Sinon, Échap arrête l'outil et le mode le relance.
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

	// --- IInteractiveToolNestedAcceptCancelAPI : Échap (tracé en cours, puis sélection) ---
	virtual bool SupportsNestedCancelCommand() override { return true; }
	virtual bool CanCurrentlyNestedCancel() override { return IsStrokeActive() || SelectedCell.IsSet(); }
	virtual bool ExecuteNestedCancelCommand() override;

	bool IsStrokeActive() const { return StrokeKind != EStrokeKind::None && !bStrokeCancelled; }

	// Annule le tracé en cours (le brouillon revient à son état d'avant le tracé)
	void CancelStroke();

	// --- Sélection ---
	void SelectCell(FIntPoint Coord);
	void ClearSelection();

	// --- Boutons du panneau ---
	void RequestSubmit();
	void RequestRevert();
	void RequestFrameBoard();
	void RequestNextError();

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
	 * Modification ponctuelle du brouillon (détails d'une case) :
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

	// --- Détails de la case sélectionnée ---

	// Recopie la case sélectionnée du brouillon dans la section « Case sélectionnée » (ou la masque).
	// bCopyCellData = false : ne met à jour que les problèmes (pendant une édition dans le panneau).
	void RefreshDetails(bool bCopyCellData = true);

	// Crée / remplace / supprime les objets de réglages pour qu'ils correspondent aux types choisis
	void FixupDetailsSettings();

	// Recopie la section « Case sélectionnée » dans le brouillon (une étape Ctrl+Z)
	void ApplyDetailsToDraft();

	// --- Affichage ---

	// Le brouillon a changé : met à jour État / Problèmes et la section « Case sélectionnée »
	void HandleDraftChanged();

	// Description et couleur du type du pinceau
	void UpdatePaintTypeInfo();

	// Affiche l'aide contextuelle correspondant à ce qui est survolé
	void UpdateHoverMessage();

	UPROPERTY()
	TObjectPtr<UBoardEditToolProperties> Properties;

	UPROPERTY()
	TObjectPtr<UBoardCellDetailsProperties> Details;

	UPROPERTY()
	TObjectPtr<UBoardEditToolHelpProperties> Help;

	TWeakObjectPtr<UBoardEditorMode> Mode;

	// Survol : case sous la souris, et lien visé s'il y en a un
	TOptional<FIntPoint> HoveredCell;
	TOptional<FBoardLink> HoveredLink;

	// Case sélectionnée (section « Case sélectionnée » du panneau)
	TOptional<FIntPoint> SelectedCell;

	// Vrai pendant que les détails sont recopiés dans le brouillon (ne pas les recharger au même moment)
	bool bApplyingDetails = false;

	// Position dans la liste des cases en erreur (bouton « Erreur suivante »)
	int32 NextErrorIndex = 0;

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
