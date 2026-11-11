#include "BoardEditTool.h"
#include "BoardDraftChange.h"
#include "PlagueTaleGOEditor/PlagueTaleGOEditor.h"
#include "PlagueTaleGOEditor/Definitions/BoardDefinitionRegistry.h"
#include "PlagueTaleGOEditor/Mode/BoardEditorMode.h"
#include "PlagueTaleGOEditor/Mode/BoardLayoutDraft.h"
#include "PlagueTaleGO/Board/Board.h"
#include "PlagueTaleGO/Level/BoardPlacementSettings.h"
#include "PlagueTaleGO/Level/Definitions/BoardNodeType.h"
#include "PlagueTaleGO/Level/Definitions/BoardEntityType.h"
#include "PlagueTaleGO/Level/Definitions/BoardItemType.h"
#include "InteractiveToolManager.h"
#include "ToolContextInterfaces.h"            // IToolsContextRenderAPI, EToolMessageLevel
#include "BaseBehaviors/ClickDragBehavior.h"
#include "BaseBehaviors/MouseHoverBehavior.h"
#include "SceneManagement.h"                  // FPrimitiveDrawInterface, DrawCircle
#include "Misc/MessageDialog.h"

namespace
{
	// Hauteur de l'aperçu au-dessus du plan du Board (cm), pour ne pas disparaître dans les meshes
	constexpr float PreviewZOffset = 5.f;

	// Zones de visée, en fraction de CellSize autour du centre d'une case :
	// en dessous de CellZone on vise la case ; au-delà, on vise le lien vers la voisine
	// si l'écart perpendiculaire reste sous LinkHalfWidth.
	constexpr double CellZone = 0.3;
	constexpr double LinkHalfWidth = 0.2;

	// Garde-fou : au-delà de cet écart entre deux positions de souris (rayon presque horizontal…),
	// on ignore le déplacement au lieu de créer une longue ligne de cases.
	constexpr int32 MaxStrokeJump = 32;

	const FLinearColor HoverColor(1.f, 0.85f, 0.f);           // jaune : case ou lien visé
	const FLinearColor SelectionColor(0.f, 0.9f, 1.f);        // cyan : case sélectionnée
	const FLinearColor ErrorColor(1.f, 0.1f, 0.1f);           // rouge : erreur, suppression
	const FLinearColor NewColor(0.2f, 1.f, 0.3f);             // vert : ajout pas encore soumis
	const FLinearColor LinkColor(0.9f, 0.9f, 0.9f);           // blanc : lien existant
	const FLinearColor MissingTypeColor(1.f, 0.f, 1.f);       // magenta : type manquant

	FText NodeTypeLabel(const UBoardNodeType* Type)
	{
		return Type ? Type->GetDisplayText() : INVTEXT("(aucun type choisi)");
	}

	FText CoordText(FIntPoint Coord)
	{
		return FText::Format(INVTEXT("({0}, {1})"), FText::AsNumber(Coord.X), FText::AsNumber(Coord.Y));
	}

	// Copie d'un objet de réglages appartenant à NewOuter (nullptr reste nullptr)
	UBoardPlacementSettings* DuplicateSettings(const UBoardPlacementSettings* Settings, UObject* NewOuter)
	{
		return Settings ? DuplicateObject<UBoardPlacementSettings>(Settings, NewOuter) : nullptr;
	}

	// Une croix au centre d'une case
	void DrawCellCross(FPrimitiveDrawInterface* PDI, const FVector& Center, float HalfSize, const FLinearColor& Color, float Thickness)
	{
		PDI->DrawLine(Center + FVector(-HalfSize, -HalfSize, 0.f), Center + FVector(HalfSize, HalfSize, 0.f), Color, SDPG_Foreground, Thickness);
		PDI->DrawLine(Center + FVector(-HalfSize, HalfSize, 0.f), Center + FVector(HalfSize, -HalfSize, 0.f), Color, SDPG_Foreground, Thickness);
	}

	// Le contour carré d'une case
	void DrawCellSquare(FPrimitiveDrawInterface* PDI, const FVector& Center, float HalfSize, const FLinearColor& Color, float Thickness)
	{
		const FVector Corners[] = {
			Center + FVector(-HalfSize, -HalfSize, 0.f), Center + FVector(HalfSize, -HalfSize, 0.f),
			Center + FVector(HalfSize, HalfSize, 0.f), Center + FVector(-HalfSize, HalfSize, 0.f) };
		for (int32 Index = 0; Index < 4; ++Index)
		{
			PDI->DrawLine(Corners[Index], Corners[(Index + 1) % 4], Color, SDPG_Foreground, Thickness);
		}
	}

	// Une flèche à plat, de Start vers End
	void DrawFlatArrow(FPrimitiveDrawInterface* PDI, const FVector& Start, const FVector& End, const FLinearColor& Color, float HeadSize, float Thickness)
	{
		PDI->DrawLine(Start, End, Color, SDPG_Foreground, Thickness);
		const FVector Dir = (End - Start).GetSafeNormal();
		const FVector Side(-Dir.Y, Dir.X, 0.f);
		PDI->DrawLine(End, End - Dir * HeadSize + Side * HeadSize * 0.6f, Color, SDPG_Foreground, Thickness);
		PDI->DrawLine(End, End - Dir * HeadSize - Side * HeadSize * 0.6f, Color, SDPG_Foreground, Thickness);
	}

	// Un petit losange (objet posé sur une case)
	void DrawDiamond(FPrimitiveDrawInterface* PDI, const FVector& Center, float HalfSize, const FLinearColor& Color, float Thickness)
	{
		const FVector Points[] = {
			Center + FVector(HalfSize, 0.f, 0.f), Center + FVector(0.f, HalfSize, 0.f),
			Center + FVector(-HalfSize, 0.f, 0.f), Center + FVector(0.f, -HalfSize, 0.f) };
		for (int32 Index = 0; Index < 4; ++Index)
		{
			PDI->DrawLine(Points[Index], Points[(Index + 1) % 4], Color, SDPG_Foreground, Thickness);
		}
	}
}

// =====================================================================================
// Builder
// =====================================================================================

bool UBoardEditToolBuilder::CanBuildTool(const FToolBuilderState& SceneState) const
{
	return Mode.IsValid() && Mode->GetDraft() && Mode->GetDraft()->GetBoard();
}

UInteractiveTool* UBoardEditToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	UBoardEditTool* Tool = NewObject<UBoardEditTool>(SceneState.ToolManager);
	Tool->Init(Mode.Get());
	return Tool;
}

// =====================================================================================
// Panneaux (boutons)
// =====================================================================================

void UBoardEditToolProperties::Submit()
{
	if (ParentTool.IsValid())
	{
		ParentTool->RequestSubmit();
	}
}

void UBoardEditToolProperties::Revert()
{
	if (ParentTool.IsValid())
	{
		ParentTool->RequestRevert();
	}
}

void UBoardEditToolProperties::NextError()
{
	if (ParentTool.IsValid())
	{
		ParentTool->RequestNextError();
	}
}

void UBoardEditToolProperties::FrameBoard()
{
	if (ParentTool.IsValid())
	{
		ParentTool->RequestFrameBoard();
	}
}

void UBoardCellDetailsProperties::Deselect()
{
	if (ParentTool.IsValid())
	{
		ParentTool->ClearSelection();
	}
}

// =====================================================================================
// Outil : mise en place
// =====================================================================================

void UBoardEditTool::Init(UBoardEditorMode* InMode)
{
	Mode = InMode;
}

void UBoardEditTool::Setup()
{
	Super::Setup();

	// Bouton gauche : clic ou glisser. Shift et Ctrl sont suivis pour choisir l'action du tracé.
	UClickDragInputBehavior* DragBehavior = NewObject<UClickDragInputBehavior>(this);
	DragBehavior->Initialize(this);
	DragBehavior->Modifiers.RegisterModifier(ShiftModifierId, FInputDeviceState::IsShiftKeyDown);
	DragBehavior->Modifiers.RegisterModifier(CtrlModifierId, FInputDeviceState::IsCtrlKeyDown);
	AddInputBehavior(DragBehavior);

	// Survol : surbrillance et aide contextuelle
	UMouseHoverBehavior* HoverBehavior = NewObject<UMouseHoverBehavior>(this);
	HoverBehavior->Initialize(this);
	AddInputBehavior(HoverBehavior);

	// Panneau 1/3 : pinceau, brouillon, vue. RestoreProperties reprend les réglages de la dernière
	// utilisation de l'outil (il est relancé après un Échap : le type du pinceau ne doit pas être perdu).
	Properties = NewObject<UBoardEditToolProperties>(this);
	Properties->RestoreProperties(this);
	Properties->ParentTool = this;
	if (!Properties->PaintType)
	{
		Properties->PaintType = FBoardDefinitionRegistry::GetDefaultNodeType();
	}
	AddToolPropertySource(Properties);

	// Panneau 2/3 : case sélectionnée (lignes cachées tant qu'aucune case n'est sélectionnée)
	Details = NewObject<UBoardCellDetailsProperties>(this);
	Details->ParentTool = this;
	Details->NoSelectionHint = INVTEXT("Aucune case sélectionnée : cliquer une case du plateau.");
	AddToolPropertySource(Details);

	// Panneau 3/3 : aide
	Help = NewObject<UBoardEditToolHelpProperties>(this);
	Help->HelpText = INVTEXT(
		"Clic gauche sur une case vide : poser une case du type choisi (elle est sélectionnée).\n"
		"Clic gauche sur une case : la sélectionner (section « Case sélectionnée » : type, entités, objets).\n"
		"Clic gauche + glisser : tracer un chemin (crée les cases et les relie ; passer sur une case existante la relie).\n"
		"Ctrl + clic gauche (ou glisser) : appliquer le type choisi aux cases.\n"
		"Shift + clic gauche sur un lien ou une case : l'effacer (une case part avec ses liens).\n"
		"Shift + clic gauche + glisser : effacer les cases survolées et leurs liens.\n"
		"Échap : annuler le tracé en cours, sinon désélectionner.\n"
		"Ctrl+Z / Ctrl+Y : annuler / rétablir dans le brouillon.\n"
		"Clic droit maintenu + ZQSD : naviguer (caméra habituelle).\n"
		"\n"
		"Départ joueur, ennemis : sélectionner la case, puis « + » dans Entités.\n"
		"Rien n'est modifié dans le niveau avant « Soumettre ».");
	AddToolPropertySource(Help);

	if (UBoardLayoutDraft* Draft = GetDraft())
	{
		DraftChangedHandle = Draft->OnChanged.AddUObject(this, &UBoardEditTool::HandleDraftChanged);
	}
	UpdatePaintTypeInfo();
	HandleDraftChanged();
	UpdateHoverMessage();
}

void UBoardEditTool::Shutdown(EToolShutdownType ShutdownType)
{
	// Un tracé interrompu par la sortie du mode ne doit pas rester à moitié appliqué
	CancelStroke();

	// Mémorise les réglages du pinceau pour la prochaine fois (voir RestoreProperties dans Setup)
	if (Properties)
	{
		Properties->SaveProperties(this);
	}

	if (UBoardLayoutDraft* Draft = GetDraft())
	{
		Draft->OnChanged.Remove(DraftChangedHandle);
	}
	GetToolManager()->DisplayMessage(FText::GetEmpty(), EToolMessageLevel::UserNotification);

	Super::Shutdown(ShutdownType);
}

UBoardLayoutDraft* UBoardEditTool::GetDraft() const
{
	return Mode.IsValid() ? Mode->GetDraft() : nullptr;
}

ABoard* UBoardEditTool::GetBoard() const
{
	const UBoardLayoutDraft* Draft = GetDraft();
	return Draft ? Draft->GetBoard() : nullptr;
}

void UBoardEditTool::OnPropertyModified(UObject* PropertySet, FProperty* Property)
{
	// Modification dans « Case sélectionnée » : l'appliquer au brouillon
	if (PropertySet == Details)
	{
		ApplyDetailsToDraft();
		return;
	}

	// Pinceau : sa description et l'aide contextuelle affichent son type
	UpdatePaintTypeInfo();
	UpdateHoverMessage();
}

// =====================================================================================
// Géométrie
// =====================================================================================

bool UBoardEditTool::RayToPoint(const FVector& Origin, const FVector& Direction, FVector& OutPoint, double& OutDepth) const
{
	const ABoard* Board = GetBoard();
	if (!Board)
	{
		return false;
	}

	// Rayon parallèle au plateau : pas d'intersection
	if (FMath::IsNearlyZero(FVector::DotProduct(Direction, FVector::UpVector)))
	{
		return false;
	}

	const FPlane BoardPlane(Board->GetActorLocation(), FVector::UpVector);
	const FVector Hit = FMath::RayPlaneIntersection(Origin, Direction, BoardPlane);
	if (FVector::DotProduct(Hit - Origin, Direction) < 0.0)
	{
		return false;   // le plan est derrière la caméra
	}

	OutPoint = Hit;
	OutDepth = FVector::Distance(Origin, Hit);
	return true;
}

bool UBoardEditTool::RayToCell(const FInputDeviceRay& Ray, FIntPoint& OutCell, double& OutDepth) const
{
	FVector Point;
	if (!RayToPoint(Ray.WorldRay.Origin, Ray.WorldRay.Direction, Point, OutDepth))
	{
		return false;
	}
	OutCell = GetBoard()->WorldToGrid(Point);
	return true;
}

void UBoardEditTool::HitTest(const FVector& Point, FIntPoint& OutCell, TOptional<FBoardLink>& OutLink) const
{
	OutLink.Reset();
	const ABoard* Board = GetBoard();
	const UBoardLayoutDraft* Draft = GetDraft();
	if (!Board || !Draft)
	{
		return;
	}

	OutCell = Board->WorldToGrid(Point);

	// Position du point par rapport au centre de sa case, en fraction de case (entre -0.5 et 0.5)
	const double Size = FMath::Max(Board->CellSize, 1.f);
	const FVector Local = (Point - Board->GridToWorld(OutCell)) / Size;
	const double AbsX = FMath::Abs(Local.X);
	const double AbsY = FMath::Abs(Local.Y);
	if (FMath::Max(AbsX, AbsY) <= CellZone)
	{
		return;   // près du centre : on vise la case
	}

	// Vers le bord : la voisine dans la direction dominante, si l'on reste dans la largeur du lien
	const bool bAlongX = AbsX >= AbsY;
	const double Perpendicular = bAlongX ? AbsY : AbsX;
	if (Perpendicular > LinkHalfWidth)
	{
		return;
	}
	const FIntPoint Neighbor = OutCell + (bAlongX
		? FIntPoint(Local.X > 0.0 ? 1 : -1, 0)
		: FIntPoint(0, Local.Y > 0.0 ? 1 : -1));

	if (Draft->GetLayout().HasLink(OutCell, Neighbor))
	{
		OutLink = FBoardLink::Make(OutCell, Neighbor);
	}
}

// =====================================================================================
// Modifications ponctuelles
// =====================================================================================

void UBoardEditTool::ApplyDraftEdit(const FText& Description, TFunctionRef<bool(FBoardLayout&)> Edit)
{
	UBoardLayoutDraft* Draft = GetDraft();
	if (!Draft)
	{
		return;
	}

	const FBoardLayout Before = Draft->GetLayout();
	FBoardLayout After = Before;
	if (!Edit(After))
	{
		return;   // rien n'a changé : pas d'étape d'historique
	}

	Draft->SetLayout(After);

	// L'éditeur rejouera Revert (Ctrl+Z) / Apply (Ctrl+Y) sur le brouillon
	GetToolManager()->EmitObjectChange(Draft, MakeUnique<FBoardDraftChange>(Before, After, Description.ToString()), Description);
}

// =====================================================================================
// Tracé (bouton gauche)
// =====================================================================================

FInputRayHit UBoardEditTool::CanBeginClickDragSequence(const FInputDeviceRay& PressPos)
{
	FIntPoint Cell = FIntPoint::ZeroValue;
	double Depth = 0.0;
	return RayToCell(PressPos, Cell, Depth) ? FInputRayHit(static_cast<float>(Depth)) : FInputRayHit();
}

void UBoardEditTool::OnClickPress(const FInputDeviceRay& PressPos)
{
	ResetStroke();

	UBoardLayoutDraft* Draft = GetDraft();
	FVector Point;
	double Depth = 0.0;
	if (!Draft || !RayToPoint(PressPos.WorldRay.Origin, PressPos.WorldRay.Direction, Point, Depth))
	{
		return;
	}

	FIntPoint Cell = FIntPoint::ZeroValue;
	TOptional<FBoardLink> Link;
	HitTest(Point, Cell, Link);

	// La nature du tracé est fixée à l'appui : Shift = effacer, Ctrl = appliquer le type, sinon tracer
	const EStrokeKind Kind = bShiftDown ? EStrokeKind::Erase : (bCtrlDown ? EStrokeKind::ApplyType : EStrokeKind::Paint);
	const bool bCellExists = Draft->GetLayout().HasCell(Cell);

	// Sans type de pinceau, on peut encore sélectionner une case existante, mais pas en créer
	if (Kind != EStrokeKind::Erase && !(Properties && Properties->PaintType) && !(Kind == EStrokeKind::Paint && bCellExists))
	{
		GetToolManager()->DisplayMessage(INVTEXT("Choisir un type de case dans le panneau (Pinceau > Type de case)."), EToolMessageLevel::UserWarning);
		return;
	}

	StrokeKind = Kind;
	StrokeStartLayout = Draft->GetLayout();
	StrokeLayout = StrokeStartLayout;
	bStrokeWasDirty = Draft->IsDirty();
	bStartCellExisted = bCellExists;
	LastStrokeCell = Cell;
	HoveredCell = Cell;
	HoveredLink = Link;

	// Shift : on attend de savoir si c'est un clic (lien ou case visé) ou un glisser (cases survolées)
	if (StrokeKind == EStrokeKind::Erase)
	{
		StrokeStartLink = Link;
		return;
	}

	if (StrokeVisit(Cell))
	{
		PushStrokeToDraft();
	}
}

void UBoardEditTool::OnClickDrag(const FInputDeviceRay& DragPos)
{
	if (!IsStrokeActive())
	{
		return;
	}

	FIntPoint Cell = FIntPoint::ZeroValue;
	double Depth = 0.0;
	if (!RayToCell(DragPos, Cell, Depth) || Cell == LastStrokeCell)
	{
		return;
	}

	const FIntPoint Delta = Cell - LastStrokeCell;
	if (FMath::Abs(Delta.X) + FMath::Abs(Delta.Y) > MaxStrokeJump)
	{
		return;   // saut aberrant (rayon presque horizontal) : ignoré
	}

	// Tracer sans type de pinceau est impossible (on avait seulement le droit de sélectionner)
	if (StrokeKind == EStrokeKind::Paint && !(Properties && Properties->PaintType))
	{
		return;
	}

	bool bChanged = false;

	// Shift + glisser : la case de départ fait partie des cases survolées
	if (StrokeKind == EStrokeKind::Erase && !bStrokeMoved)
	{
		bChanged |= StrokeVisit(LastStrokeCell);
	}

	bStrokeMoved = true;
	HoveredCell = Cell;
	HoveredLink.Reset();

	if (StrokeKind == EStrokeKind::Paint)
	{
		// Chemin de case en case, sans diagonale : on avance d'abord sur l'axe le plus long
		FIntPoint Current = LastStrokeCell;
		while (Current != Cell)
		{
			const FIntPoint Remaining = Cell - Current;
			const FIntPoint Step = FMath::Abs(Remaining.X) >= FMath::Abs(Remaining.Y)
				? FIntPoint(FMath::Sign(Remaining.X), 0)
				: FIntPoint(0, FMath::Sign(Remaining.Y));
			const FIntPoint Next = Current + Step;
			bChanged |= PaintStep(Current, Next);
			Current = Next;
		}
	}
	else
	{
		// Effacer / appliquer le type : seulement les cases réellement survolées
		bChanged |= StrokeVisit(Cell);
	}

	LastStrokeCell = Cell;
	if (bChanged)
	{
		PushStrokeToDraft();
	}
}

void UBoardEditTool::OnClickRelease(const FInputDeviceRay& ReleasePos)
{
	UBoardLayoutDraft* Draft = GetDraft();
	if (StrokeKind == EStrokeKind::None || bStrokeCancelled || !Draft)
	{
		ResetStroke();
		return;
	}

	const EStrokeKind Kind = StrokeKind;
	const bool bWasClick = !bStrokeMoved;
	const FIntPoint ClickedCell = LastStrokeCell;

	// Shift + clic sans bouger : le lien visé est prioritaire, sinon la case
	if (Kind == EStrokeKind::Erase && bWasClick)
	{
		const bool bErased = StrokeStartLink.IsSet()
			? StrokeLayout.RemoveLink(StrokeStartLink->A, StrokeStartLink->B)
			: StrokeLayout.RemoveCell(ClickedCell);
		if (bErased)
		{
			bStrokeChanged = true;
			PushStrokeToDraft();
		}
	}

	// Le tracé complet forme UNE étape d'historique
	if (bStrokeChanged)
	{
		FText Description;
		switch (Kind)
		{
		case EStrokeKind::Erase:
			Description = !bWasClick ? INVTEXT("Plateau : effacer des cases")
				: (StrokeStartLink.IsSet() ? INVTEXT("Plateau : effacer un lien") : INVTEXT("Plateau : effacer une case"));
			break;
		case EStrokeKind::ApplyType:
			Description = INVTEXT("Plateau : appliquer un type");
			break;
		default:
			Description = !bWasClick ? INVTEXT("Plateau : tracer un chemin") : INVTEXT("Plateau : poser une case");
			break;
		}
		GetToolManager()->EmitObjectChange(Draft, MakeUnique<FBoardDraftChange>(StrokeStartLayout, StrokeLayout, Description.ToString()), Description);
	}

	ResetStroke();

	// Clic simple (sans touche) : sélectionner la case cliquée, qu'elle existait ou vienne d'être posée
	if (Kind == EStrokeKind::Paint && bWasClick)
	{
		SelectCell(ClickedCell);
	}

	UpdateHoverMessage();
}

void UBoardEditTool::OnTerminateDragSequence()
{
	// L'éditeur a interrompu le tracé (perte de focus…) : on l'annule proprement
	CancelStroke();
	ResetStroke();
}

bool UBoardEditTool::ExecuteNestedCancelCommand()
{
	// Échap : d'abord le tracé en cours, sinon la sélection
	if (IsStrokeActive())
	{
		CancelStroke();
		return true;
	}
	if (SelectedCell.IsSet())
	{
		ClearSelection();
		return true;
	}
	return false;
}

void UBoardEditTool::CancelStroke()
{
	if (!IsStrokeActive())
	{
		return;
	}

	if (bStrokeChanged)
	{
		if (UBoardLayoutDraft* Draft = GetDraft())
		{
			Draft->CancelTo(StrokeStartLayout, bStrokeWasDirty);
		}
	}

	// Les déplacements suivants sont ignorés jusqu'au relâchement du bouton
	bStrokeCancelled = true;
	GetToolManager()->DisplayMessage(INVTEXT("Tracé annulé."), EToolMessageLevel::UserNotification);
}

bool UBoardEditTool::StrokeVisit(FIntPoint Cell)
{
	UBoardNodeType* PaintType = Properties ? Properties->PaintType.Get() : nullptr;
	bool bChanged = false;

	switch (StrokeKind)
	{
	case EStrokeKind::Paint:
		// Départ du tracé : poser la case si elle n'existe pas
		if (PaintType && !StrokeLayout.HasCell(Cell))
		{
			FBoardCell& NewCell = StrokeLayout.AddCell(Cell, PaintType);
			NewCell.NodeSettings = PaintType->CreateSettings(GetDraft());
			bChanged = true;
		}
		break;

	case EStrokeKind::ApplyType:
		if (FBoardCell* Target = StrokeLayout.FindCell(Cell))
		{
			if (PaintType && Target->NodeType != PaintType)
			{
				Target->NodeType = PaintType;
				// Nouveaux réglages pour le nouveau type (vides si le type n'en a pas)
				Target->NodeSettings = PaintType->CreateSettings(GetDraft());
				bChanged = true;
			}
		}
		break;

	case EStrokeKind::Erase:
		bChanged = StrokeLayout.RemoveCell(Cell);
		break;

	default:
		break;
	}

	bStrokeChanged |= bChanged;
	return bChanged;
}

bool UBoardEditTool::PaintStep(FIntPoint From, FIntPoint To)
{
	UBoardNodeType* PaintType = Properties ? Properties->PaintType.Get() : nullptr;
	bool bChanged = false;

	// Case d'arrivée : créée si besoin (une case existante est simplement reliée)
	if (PaintType && !StrokeLayout.HasCell(To))
	{
		FBoardCell& NewCell = StrokeLayout.AddCell(To, PaintType);
		NewCell.NodeSettings = PaintType->CreateSettings(GetDraft());
		bChanged = true;
	}

	// Lien avec la case précédente (AddLink refuse un lien déjà existant ou invalide)
	bChanged |= StrokeLayout.AddLink(From, To);

	bStrokeChanged |= bChanged;
	return bChanged;
}

void UBoardEditTool::PushStrokeToDraft()
{
	if (UBoardLayoutDraft* Draft = GetDraft())
	{
		Draft->SetLayout(StrokeLayout);
	}
}

void UBoardEditTool::ResetStroke()
{
	StrokeKind = EStrokeKind::None;
	bStrokeChanged = false;
	bStrokeMoved = false;
	bStrokeCancelled = false;
	bStartCellExisted = false;
	StrokeStartLink.Reset();
}

// =====================================================================================
// Sélection et détails
// =====================================================================================

void UBoardEditTool::SelectCell(FIntPoint Coord)
{
	const UBoardLayoutDraft* Draft = GetDraft();
	if (!Draft || !Draft->GetLayout().HasCell(Coord))
	{
		ClearSelection();
		return;
	}
	SelectedCell = Coord;
	RefreshDetails();
}

void UBoardEditTool::ClearSelection()
{
	SelectedCell.Reset();
	RefreshDetails();
}

void UBoardEditTool::RefreshDetails(bool bCopyCellData)
{
	const UBoardLayoutDraft* Draft = GetDraft();
	const FBoardCell* Cell = (Draft && SelectedCell.IsSet()) ? Draft->GetLayout().FindCell(SelectedCell.GetValue()) : nullptr;
	if (!Details)
	{
		return;
	}
	if (!Cell)
	{
		// Case effacée, annulée (Ctrl+Z) ou rien de sélectionné : les lignes de la section se cachent
		SelectedCell.Reset();
		Details->bHasSelection = false;
		NotifyOfPropertyChangeByTool(Details);
		return;
	}

	// Copies des réglages : le panneau ne modifie jamais directement le brouillon (tout passe par l'historique)
	if (bCopyCellData)
	{
		Details->Coord = Cell->Coord;
		Details->NodeType = Cell->NodeType;
		Details->TypeDescription = Cell->NodeType ? Cell->NodeType->Description : FText::GetEmpty();
		Details->NodeSettings = DuplicateSettings(Cell->NodeSettings, Details);
		Details->Entities = Cell->Entities;
		for (FBoardEntityPlacement& Entity : Details->Entities)
		{
			Entity.Settings = DuplicateSettings(Entity.Settings, Details);
		}
		Details->Items = Cell->Items;
		for (FBoardItemPlacement& Item : Details->Items)
		{
			Item.Settings = DuplicateSettings(Item.Settings, Details);
		}
	}

	Details->CellProblems.Reset();
	for (const FBoardValidationMessage& Message : Draft->GetValidation().Messages)
	{
		if (Message.Cells.Contains(Cell->Coord))
		{
			const TCHAR* Prefix = Message.Severity == EBoardValidationSeverity::Error ? TEXT("Erreur : ") : TEXT("Avertissement : ");
			Details->CellProblems.Add(Prefix + Message.Text.ToString());
		}
	}

	Details->bHasSelection = true;
	NotifyOfPropertyChangeByTool(Details);
}

void UBoardEditTool::FixupDetailsSettings()
{
	// Chaque élément doit avoir exactement un objet de réglages de la classe demandée par son type (ou aucun)
	auto Fix = [this](const UBoardDefinition* Definition, TObjectPtr<UBoardPlacementSettings>& Settings)
	{
		UClass* Wanted = nullptr;
		if (Definition && Definition->SettingsClass && !Definition->SettingsClass->HasAnyClassFlags(CLASS_Abstract))
		{
			Wanted = Definition->SettingsClass.Get();
		}
		if (!Wanted)
		{
			Settings = nullptr;
		}
		else if (!Settings || Settings->GetClass() != Wanted)
		{
			Settings = Definition->CreateSettings(Details);
		}
	};

	Fix(Details->NodeType, Details->NodeSettings);
	for (FBoardEntityPlacement& Entity : Details->Entities)
	{
		Fix(Entity.Type, Entity.Settings);
	}
	for (FBoardItemPlacement& Item : Details->Items)
	{
		Fix(Item.Type, Item.Settings);
	}
}

void UBoardEditTool::ApplyDetailsToDraft()
{
	if (!Details || !SelectedCell.IsSet())
	{
		return;
	}

	FixupDetailsSettings();
	Details->TypeDescription = Details->NodeType ? Details->NodeType->Description : FText::GetEmpty();

	// Pendant l'application, ne pas recharger le panneau depuis le brouillon (l'édition en cours serait perturbée)
	{
		TGuardValue<bool> Guard(bApplyingDetails, true);

		const FIntPoint Coord = SelectedCell.GetValue();
		ApplyDraftEdit(INVTEXT("Plateau : modifier une case"), [this, Coord](FBoardLayout& Layout)
		{
			FBoardCell* Cell = Layout.FindCell(Coord);
			UBoardLayoutDraft* Draft = GetDraft();
			if (!Cell || !Draft)
			{
				return false;
			}

			// Le brouillon garde ses propres copies des réglages
			Cell->NodeType = Details->NodeType;
			Cell->NodeSettings = DuplicateSettings(Details->NodeSettings, Draft);
			Cell->Entities = Details->Entities;
			for (FBoardEntityPlacement& Entity : Cell->Entities)
			{
				Entity.Settings = DuplicateSettings(Entity.Settings, Draft);
			}
			Cell->Items = Details->Items;
			for (FBoardItemPlacement& Item : Cell->Items)
			{
				Item.Settings = DuplicateSettings(Item.Settings, Draft);
			}
			return true;
		});
	}

	// Problèmes de la case, description du type, réglages créés : le panneau doit les afficher
	// (sans recopier la case : l'utilisateur est en train d'éditer ces valeurs)
	RefreshDetails(/*bCopyCellData*/ false);
}

// =====================================================================================
// Survol
// =====================================================================================

FInputRayHit UBoardEditTool::BeginHoverSequenceHitTest(const FInputDeviceRay& PressPos)
{
	FIntPoint Cell = FIntPoint::ZeroValue;
	double Depth = 0.0;
	return RayToCell(PressPos, Cell, Depth) ? FInputRayHit(static_cast<float>(Depth)) : FInputRayHit();
}

void UBoardEditTool::OnBeginHover(const FInputDeviceRay& DevicePos)
{
	OnUpdateHover(DevicePos);
}

bool UBoardEditTool::OnUpdateHover(const FInputDeviceRay& DevicePos)
{
	TOptional<FIntPoint> NewCell;
	TOptional<FBoardLink> NewLink;

	FVector Point;
	double Depth = 0.0;
	if (RayToPoint(DevicePos.WorldRay.Origin, DevicePos.WorldRay.Direction, Point, Depth))
	{
		FIntPoint Cell = FIntPoint::ZeroValue;
		HitTest(Point, Cell, NewLink);
		NewCell = Cell;
	}

	// Ne réécrire l'aide que si la cible change
	if (NewCell != HoveredCell || NewLink != HoveredLink)
	{
		HoveredCell = NewCell;
		HoveredLink = NewLink;
		UpdateHoverMessage();
	}
	return true;   // continuer à suivre le survol
}

void UBoardEditTool::OnEndHover()
{
	HoveredCell.Reset();
	HoveredLink.Reset();
	UpdateHoverMessage();
}

void UBoardEditTool::OnUpdateModifierState(int ModifierID, bool bIsOn)
{
	if (ModifierID == ShiftModifierId)
	{
		bShiftDown = bIsOn;
	}
	else if (ModifierID == CtrlModifierId)
	{
		bCtrlDown = bIsOn;
	}
}

void UBoardEditTool::UpdateHoverMessage()
{
	const UBoardLayoutDraft* Draft = GetDraft();
	const FText PaintName = NodeTypeLabel(Properties ? Properties->PaintType.Get() : nullptr);

	FText Message;
	if (!Draft || !HoveredCell.IsSet())
	{
		Message = INVTEXT("Survoler le plateau pour voir les actions possibles. Rien n'est modifié avant « Soumettre ».");
	}
	else if (HoveredLink.IsSet())
	{
		Message = FText::Format(INVTEXT("Lien {0} – {1} — Shift + clic : effacer le lien"),
			CoordText(HoveredLink->A), CoordText(HoveredLink->B));
	}
	else if (const FBoardCell* Cell = Draft->GetLayout().FindCell(HoveredCell.GetValue()))
	{
		Message = FText::Format(INVTEXT("Case {0} « {1} » — Clic : sélectionner · Glisser : tracer depuis cette case · Ctrl + clic : appliquer « {2} » · Shift + clic : effacer"),
			CoordText(Cell->Coord), NodeTypeLabel(Cell->NodeType), PaintName);
	}
	else
	{
		Message = FText::Format(INVTEXT("Case vide {0} — Clic : poser « {1} » · Glisser : tracer un chemin"),
			CoordText(HoveredCell.GetValue()), PaintName);
	}

	GetToolManager()->DisplayMessage(Message, EToolMessageLevel::UserNotification);
}

// =====================================================================================
// Panneau
// =====================================================================================

void UBoardEditTool::UpdatePaintTypeInfo()
{
	if (!Properties)
	{
		return;
	}
	const UBoardNodeType* Type = Properties->PaintType;
	Properties->PaintTypeDescription = Type ? Type->Description : INVTEXT("Aucun type choisi : impossible de poser des cases.");
	Properties->PaintTypeColor = Type ? Type->EditorColor : MissingTypeColor;
	NotifyOfPropertyChangeByTool(Properties);
}

void UBoardEditTool::HandleDraftChanged()
{
	const UBoardLayoutDraft* Draft = GetDraft();
	if (!Properties || !Draft)
	{
		return;
	}

	const FBoardLayout& Layout = Draft->GetLayout();
	const FBoardValidationResult& Validation = Draft->GetValidation();

	Properties->Status = FString::Printf(TEXT("%s — %d case(s), %d lien(s) — %d erreur(s), %d avertissement(s)"),
		Draft->IsDirty() ? TEXT("Modifié, non soumis") : TEXT("À jour"),
		Layout.Cells.Num(), Layout.Links.Num(), Validation.CountErrors(), Validation.CountWarnings());

	Properties->Problems.Reset();
	for (const FBoardValidationMessage& Message : Validation.Messages)
	{
		const TCHAR* Prefix = Message.Severity == EBoardValidationSeverity::Error ? TEXT("Erreur : ") : TEXT("Avertissement : ");
		Properties->Problems.Add(Prefix + Message.Text.ToString());
	}

	// Le panneau ne se rafraîchit pas tout seul quand le code modifie des propriétés : on le prévient
	NotifyOfPropertyChangeByTool(Properties);

	// La case sélectionnée a pu changer (Ctrl+Z, effacement…) : sauf si le changement vient du panneau lui-même
	if (!bApplyingDetails)
	{
		RefreshDetails();
	}
	UpdateHoverMessage();
}

void UBoardEditTool::RequestSubmit()
{
	UBoardLayoutDraft* Draft = GetDraft();
	if (!Mode.IsValid() || !Draft)
	{
		return;
	}

	const bool bIgnoreErrors = Properties && Properties->bIgnoreErrors;
	if (Mode->SubmitDraft(bIgnoreErrors))
	{
		GetToolManager()->DisplayMessage(INVTEXT("Brouillon soumis : le niveau est à jour (sauvegarder le niveau pour le conserver)."), EToolMessageLevel::UserNotification);
		return;
	}

	FMessageDialog::Open(EAppMsgType::Ok, FText::Format(
		INVTEXT("Submit refusé : le brouillon contient {0} erreur(s).\n\nVoir la liste « Problèmes » du panneau (bouton « Erreur suivante »), ou cocher « Ignorer les erreurs (dev) »."),
		FText::AsNumber(Draft->GetValidation().CountErrors())));
}

void UBoardEditTool::RequestRevert()
{
	UBoardLayoutDraft* Draft = GetDraft();
	if (!Mode.IsValid() || !Draft || !Draft->IsDirty())
	{
		return;
	}

	const EAppReturnType::Type Answer = FMessageDialog::Open(EAppMsgType::YesNo,
		INVTEXT("Abandonner toutes les modifications non soumises du brouillon ?"));
	if (Answer == EAppReturnType::Yes)
	{
		Mode->RevertDraft();
	}
}

void UBoardEditTool::RequestFrameBoard()
{
	if (Mode.IsValid())
	{
		Mode->FrameBoardFromTop();
	}
}

void UBoardEditTool::RequestNextError()
{
	const UBoardLayoutDraft* Draft = GetDraft();
	if (!Mode.IsValid() || !Draft)
	{
		return;
	}

	// Une entrée par (case, message) : erreurs d'abord, puis avertissements. Les messages sans case
	// (ex. « aucune sortie ») ne peuvent pas être montrés sur le plateau : ils restent dans « Problèmes ».
	struct FProblemSpot
	{
		FIntPoint Coord;
		FText Text;
		bool bIsError;
	};
	TArray<FProblemSpot> Spots;
	for (const EBoardValidationSeverity Severity : { EBoardValidationSeverity::Error, EBoardValidationSeverity::Warning })
	{
		for (const FBoardValidationMessage& Message : Draft->GetValidation().Messages)
		{
			if (Message.Severity != Severity)
			{
				continue;
			}
			for (const FIntPoint& Coord : Message.Cells)
			{
				Spots.Add({ Coord, Message.Text, Severity == EBoardValidationSeverity::Error });
			}
		}
	}

	if (Spots.Num() == 0)
	{
		GetToolManager()->DisplayMessage(INVTEXT("Aucun problème lié à une case (voir aussi la liste « Problèmes »)."), EToolMessageLevel::UserNotification);
		return;
	}

	const FProblemSpot& Spot = Spots[NextErrorIndex % Spots.Num()];
	NextErrorIndex = (NextErrorIndex + 1) % Spots.Num();

	SelectCell(Spot.Coord);
	Mode->FocusCell(Spot.Coord);
	GetToolManager()->DisplayMessage(FText::Format(INVTEXT("{0} {1} : {2}"),
		Spot.bIsError ? INVTEXT("Erreur") : INVTEXT("Avertissement"), CoordText(Spot.Coord), Spot.Text),
		EToolMessageLevel::UserWarning);
}

// =====================================================================================
// Aperçu
// =====================================================================================

void UBoardEditTool::Render(IToolsContextRenderAPI* RenderAPI)
{
	const UBoardLayoutDraft* Draft = GetDraft();
	const ABoard* Board = GetBoard();
	FPrimitiveDrawInterface* PDI = RenderAPI ? RenderAPI->GetPrimitiveDrawInterface() : nullptr;
	if (!Draft || !Board || !PDI)
	{
		return;
	}

	const FBoardLayout& Layout = Draft->GetLayout();     // ce qu'on édite
	const FBoardLayout& Current = Board->GetLayout();    // ce qui est dans le niveau
	const float Size = Board->CellSize;
	const FVector Lift(0.f, 0.f, PreviewZOffset);

	auto CellCenter = [Board, &Lift](FIntPoint Coord)
	{
		return Board->GridToWorld(Coord) + Lift;
	};

	// 1. Liens du brouillon : jaune si visé, vert si ajouté (pas encore soumis), blanc sinon
	for (const FBoardLink& Link : Layout.Links)
	{
		const bool bHovered = HoveredLink.IsSet() && HoveredLink.GetValue() == Link;
		const FLinearColor Color = bHovered ? HoverColor : (Current.HasLink(Link.A, Link.B) ? LinkColor : NewColor);
		PDI->DrawLine(CellCenter(Link.A), CellCenter(Link.B), Color, SDPG_Foreground, bHovered ? 6.f : 3.f);
	}

	// 2. Liens qui seront supprimés au Submit : rouge
	for (const FBoardLink& Link : Current.Links)
	{
		if (!Layout.HasLink(Link.A, Link.B))
		{
			PDI->DrawLine(CellCenter(Link.A), CellCenter(Link.B), ErrorColor, SDPG_Foreground, 1.5f);
		}
	}

	// 3. Cases du brouillon : cercle de la couleur du type ; vert autour si ajoutée ; rouge autour si en erreur.
	//    Entités : une flèche dans la direction où elles regardent. Objets : un petit losange.
	for (const FBoardCell& Cell : Layout.Cells)
	{
		const FVector Center = CellCenter(Cell.Coord);
		const FLinearColor TypeColor = Cell.NodeType ? Cell.NodeType->EditorColor : MissingTypeColor;
		DrawCircle(PDI, Center, FVector::ForwardVector, FVector::RightVector, TypeColor, Size * 0.25f, 24, SDPG_Foreground, 4.f);

		if (!Current.HasCell(Cell.Coord))
		{
			DrawCircle(PDI, Center, FVector::ForwardVector, FVector::RightVector, NewColor, Size * 0.32f, 24, SDPG_Foreground, 2.f);
		}
		if (Draft->GetErrorCells().Contains(Cell.Coord))
		{
			DrawCircle(PDI, Center, FVector::ForwardVector, FVector::RightVector, ErrorColor, Size * 0.4f, 24, SDPG_Foreground, 2.f);
		}

		for (int32 Index = 0; Index < Cell.Entities.Num(); ++Index)
		{
			const FBoardEntityPlacement& Entity = Cell.Entities[Index];
			const FIntPoint Offset = BoardDirection::ToGridOffset(Entity.Facing);
			const FVector Dir(Offset.X, Offset.Y, 0.f);
			// Plusieurs entités : légèrement décalées pour rester lisibles
			const FVector Start = Center + FVector(0.f, 0.f, 2.f * Index);
			const FLinearColor Color = (Entity.Type && Entity.Type->PawnClass) ? Entity.Type->EditorColor : MissingTypeColor;
			DrawFlatArrow(PDI, Start, Start + Dir * Size * 0.22f, Color, Size * 0.08f, 3.f);
		}

		for (int32 Index = 0; Index < Cell.Items.Num(); ++Index)
		{
			const FBoardItemPlacement& Item = Cell.Items[Index];
			const FVector ItemCenter = Center + FVector(-Size * 0.15f, -Size * 0.15f + Index * Size * 0.08f, 0.f);
			const FLinearColor Color = Item.Type ? Item.Type->EditorColor : MissingTypeColor;
			DrawDiamond(PDI, ItemCenter, Size * 0.04f, Color, 2.f);
		}
	}

	// 4. Cases qui seront supprimées au Submit : croix rouge
	for (const FBoardCell& Cell : Current.Cells)
	{
		if (!Layout.HasCell(Cell.Coord))
		{
			DrawCellCross(PDI, CellCenter(Cell.Coord), Size * 0.25f, ErrorColor, 3.f);
		}
	}

	// 5. Case sélectionnée, puis case survolée (sauf si c'est un lien qui est visé)
	if (SelectedCell.IsSet())
	{
		DrawCellSquare(PDI, CellCenter(SelectedCell.GetValue()), Size * 0.45f, SelectionColor, 4.f);
	}
	if (HoveredCell.IsSet() && !HoveredLink.IsSet())
	{
		DrawCellSquare(PDI, CellCenter(HoveredCell.GetValue()), Size * 0.5f, HoverColor, 2.f);
	}
}
