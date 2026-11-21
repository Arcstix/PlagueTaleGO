#include "BoardEditorMode.h"
#include "BoardEditorModeCommands.h"
#include "BoardEditorInputProcessor.h"
#include "BoardEditorModeToolkit.h"
#include "BoardLayoutDraft.h"
#include "PlagueTaleGOEditor/PlagueTaleGOEditor.h"
#include "PlagueTaleGOEditor/Sync/BoardLayoutSync.h"
#include "PlagueTaleGOEditor/Tools/BoardEditTool.h"
#include "PlagueTaleGO/Board/Board.h"
#include "PlagueTaleGO/Level/Validation/BoardValidator.h"
#include "Editor.h"                  // GEditor, FEditorDelegates
#include "LevelEditorViewport.h"     // FLevelEditorViewportClient, GCurrentLevelEditingViewportClient
#include "UnrealClient.h"            // FViewport
#include "InteractiveToolManager.h"
#include "Misc/MessageDialog.h"
#include "Framework/Application/SlateApplication.h"

// Textes affichés à l'utilisateur : en anglais, en LOCTEXT (convention UE pour tout FText affiché)
#define LOCTEXT_NAMESPACE "BoardEditorMode"

const FEditorModeID UBoardEditorMode::ModeId = TEXT("EM_BoardEditorMode");
const FString UBoardEditorMode::EditToolName = TEXT("BoardEditTool");

UBoardEditorMode::UBoardEditorMode()
{
	// Nom et visibilité dans le menu des modes de l'éditeur
	Info = FEditorModeInfo(ModeId, LOCTEXT("ModeName", "Board Editor"), FSlateIcon(), /*bIsVisible*/ true);
}

void UBoardEditorMode::Enter()
{
	// Crée le panneau (toolkit) et le contexte des outils
	Super::Enter();

	bIsExiting = false;
	bRestartToolPending = false;

	Draft = NewObject<UBoardLayoutDraft>(this);

	// L'outil est enregistré même sans Board : son bouton existe, mais il ne démarre pas (CanBuildTool)
	UBoardEditToolBuilder* Builder = NewObject<UBoardEditToolBuilder>(this);
	Builder->Mode = this;
	RegisterTool(FBoardEditorModeCommands::Get().EditTool, EditToolName, Builder);

	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	ABoard* Board = FBoardLayoutSync::FindBoard(World);
	if (!Board)
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			LOCTEXT("NoBoard", "No BP_Board in this level.\n\nPlace a BP_Board (grid origin), then reopen the \"Board Editor\" mode."));
		return;
	}

	Draft->ResetFromBoard(Board);

	// Vue d'ensemble : caméra à la verticale du plateau (la vue d'avant est rétablie à la sortie)
	SaveCamera();
	FrameBoardFromTop();

	StartEditTool();

	MapLoadHandle = FEditorDelegates::OnMapLoad.AddUObject(this, &UBoardEditorMode::HandleMapLoad);

	// Échap doit atteindre l'outil avant les commandes de l'éditeur
	if (FSlateApplication::IsInitialized())
	{
		InputProcessor = MakeShared<FBoardEditorInputProcessor>(this);
		FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
	}
}

void UBoardEditorMode::Exit()
{
	bIsExiting = true;

	FEditorDelegates::OnMapLoad.Remove(MapLoadHandle);
	MapLoadHandle.Reset();

	if (InputProcessor.IsValid() && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
	}
	InputProcessor.Reset();

	// Avant Super::Exit, qui arrête l'outil : le Board et le brouillon sont encore là
	AskAboutPendingDraft();

	Super::Exit();
	Draft = nullptr;

	RestoreCamera();
}

void UBoardEditorMode::CreateToolkit()
{
	Toolkit = MakeShareable(new FBoardEditorModeToolkit);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> UBoardEditorMode::GetModeCommands() const
{
	return FBoardEditorModeCommands::GetCommands();
}

void UBoardEditorMode::StartEditTool()
{
	GetToolManager()->SelectActiveToolType(EToolSide::Left, EditToolName);
	GetToolManager()->ActivateTool(EToolSide::Left);
}

void UBoardEditorMode::OnToolEnded(UInteractiveToolManager* Manager, UInteractiveTool* Tool)
{
	Super::OnToolEnded(Manager, Tool);

	// Échap (hors tracé) ou le bouton de la barre d'outils arrêtent l'outil. Le relancer tout de suite,
	// pendant son propre arrêt, serait risqué : on le fait au prochain tick du mode.
	if (!bIsExiting && Cast<UBoardEditTool>(Tool))
	{
		bRestartToolPending = true;
	}
}

void UBoardEditorMode::ModeTick(float DeltaTime)
{
	Super::ModeTick(DeltaTime);

	if (bRestartToolPending && !bIsExiting)
	{
		bRestartToolPending = false;
		if (Draft && Draft->GetBoard() && !GetToolManager()->HasActiveTool(EToolSide::Left))
		{
			StartEditTool();
		}
	}
}

UBoardEditTool* UBoardEditorMode::GetEditTool() const
{
	return Cast<UBoardEditTool>(GetToolManager()->GetActiveTool(EToolSide::Left));
}

FLevelEditorViewportClient* UBoardEditorMode::FindPerspectiveViewport()
{
	if (GCurrentLevelEditingViewportClient && GCurrentLevelEditingViewportClient->IsPerspective())
	{
		return GCurrentLevelEditingViewportClient;
	}
	if (GEditor)
	{
		for (FLevelEditorViewportClient* Client : GEditor->GetLevelViewportClients())
		{
			if (Client && Client->IsPerspective())
			{
				return Client;
			}
		}
	}
	return nullptr;
}

void UBoardEditorMode::FrameBoardFromTop()
{
	const ABoard* Board = Draft ? Draft->GetBoard() : nullptr;
	FLevelEditorViewportClient* Client = FindPerspectiveViewport();
	if (!Board || !Client || !Client->Viewport)
	{
		return;
	}

	// Rectangle englobant les cases du brouillon (plateau vide : autour de l'origine du Board)
	FBox2D Bounds(ForceInit);
	for (const FBoardCell& Cell : Draft->GetLayout().Cells)
	{
		const FVector Center = Board->GridToWorld(Cell.Coord);
		Bounds += FVector2D(Center.X, Center.Y);
	}
	if (!Bounds.bIsValid)
	{
		const FVector Origin = Board->GetActorLocation();
		Bounds += FVector2D(Origin.X, Origin.Y);
	}

	// Une case de marge tout autour, et au minimum quelques cases visibles
	const double Margin = Board->CellSize;
	const double HalfX = FMath::Max(Bounds.GetExtent().X + Margin, Board->CellSize * 2.0);
	const double HalfY = FMath::Max(Bounds.GetExtent().Y + Margin, Board->CellSize * 2.0);

	// Caméra regardant vers le bas, Yaw 0 : l'axe +X du monde (Nord) est en haut de l'écran, +Y (Est) à droite.
	// ViewFOV est l'angle horizontal ; l'angle vertical dépend du format de l'image.
	const FIntPoint ViewSize = Client->Viewport->GetSizeXY();
	const double Aspect = ViewSize.Y > 0 ? static_cast<double>(ViewSize.X) / ViewSize.Y : 16.0 / 9.0;
	const double TanHalfHorizontal = FMath::Tan(FMath::DegreesToRadians(Client->ViewFOV * 0.5));
	const double TanHalfVertical = TanHalfHorizontal / Aspect;

	// Hauteur pour que tout tienne : Y du monde sur la largeur de l'écran, X du monde sur sa hauteur
	const double Height = FMath::Max(HalfY / TanHalfHorizontal, HalfX / TanHalfVertical);

	const FVector2D Center = Bounds.GetCenter();
	Client->SetViewLocation(FVector(Center.X, Center.Y, Board->GetActorLocation().Z + Height));
	Client->SetViewRotation(FRotator(-90.f, 0.f, 0.f));
	Client->Invalidate();
}

void UBoardEditorMode::FocusCell(FIntPoint Coord)
{
	const ABoard* Board = Draft ? Draft->GetBoard() : nullptr;
	FLevelEditorViewportClient* Client = FindPerspectiveViewport();
	if (!Board || !Client)
	{
		return;
	}

	const FVector Target = Board->GridToWorld(Coord);
	const FVector Forward = Client->GetViewRotation().Vector();

	// Caméra qui ne regarde pas vers le plateau : on repasse en vue de dessus avant de centrer
	if (Forward.Z > -0.1)
	{
		FrameBoardFromTop();
	}

	// On garde l'orientation et la distance au plan du plateau, et on glisse la caméra
	// pour que le centre de l'écran tombe sur la case
	const FVector CameraLocation = Client->GetViewLocation();
	const FVector ViewForward = Client->GetViewRotation().Vector();
	const double Distance = (CameraLocation.Z - Target.Z) / FMath::Max(-ViewForward.Z, 0.1);
	Client->SetViewLocation(Target - ViewForward * Distance);
	Client->Invalidate();
}

bool UBoardEditorMode::HandleEscape()
{
	UBoardEditTool* Tool = GetEditTool();
	if (!Tool)
	{
		return false;
	}

	// Seulement quand le viewport a le focus : Échap dans un champ du panneau garde son rôle habituel
	const FLevelEditorViewportClient* Client = GCurrentLevelEditingViewportClient;
	if (!Client || !Client->Viewport || !Client->Viewport->HasFocus())
	{
		return false;
	}

	return Tool->ExecuteNestedCancelCommand();
}

void UBoardEditorMode::SaveCamera()
{
	SavedViewportClient = FindPerspectiveViewport();
	if (SavedViewportClient)
	{
		SavedViewLocation = SavedViewportClient->GetViewLocation();
		SavedViewRotation = SavedViewportClient->GetViewRotation();
	}
}

void UBoardEditorMode::RestoreCamera()
{
	// Le viewport a pu être fermé entre-temps (changement de disposition) : on vérifie qu'il existe encore
	if (SavedViewportClient && GEditor && GEditor->GetLevelViewportClients().Contains(SavedViewportClient))
	{
		SavedViewportClient->SetViewLocation(SavedViewLocation);
		SavedViewportClient->SetViewRotation(SavedViewRotation);
		SavedViewportClient->Invalidate();
	}
	SavedViewportClient = nullptr;
}

bool UBoardEditorMode::SubmitDraft(bool bIgnoreErrors)
{
	ABoard* Board = Draft ? Draft->GetBoard() : nullptr;
	if (!Board)
	{
		return false;
	}

	const FBoardValidationResult& Result = Draft->GetValidation();
	if (Result.HasErrors())
	{
		if (!bIgnoreErrors)
		{
			FBoardValidator::LogResult(Result, TEXT("Submit refusé"));
			return false;
		}
		UE_LOG(LogBoardEditor, Warning, TEXT("[BoardEditor] Submit forcé malgré %d erreur(s) (option « Ignore Errors »)."),
			Result.CountErrors());
	}

	FBoardLayoutSync::Submit(*Board, Draft->GetLayout(), LOCTEXT("SubmitTransaction", "Board: Submit"));

	// Le brouillon repart de ce qui vient d'être écrit : il n'est plus « modifié »
	Draft->ResetFromBoard(Board);

	UE_LOG(LogBoardEditor, Log, TEXT("[BoardEditor] Brouillon soumis : %d case(s), %d lien(s)."),
		Board->GetLayout().Cells.Num(), Board->GetLayout().Links.Num());
	return true;
}

void UBoardEditorMode::RevertDraft()
{
	if (Draft)
	{
		Draft->ResetFromBoard(Draft->GetBoard());
	}
}

void UBoardEditorMode::HandleMapLoad(const FString& Filename, FCanLoadMap& OutCanLoadMap)
{
	AskAboutPendingDraft();
}

void UBoardEditorMode::AskAboutPendingDraft()
{
	if (!Draft || !Draft->IsDirty() || !Draft->GetBoard())
	{
		return;
	}

	const EAppReturnType::Type Answer = FMessageDialog::Open(EAppMsgType::YesNo,
		LOCTEXT("PendingDraft", "The board draft has unsubmitted changes.\n\nYes: submit them now.\nNo: discard them."));

	if (Answer == EAppReturnType::Yes && !SubmitDraft(/*bIgnoreErrors*/ false))
	{
		const EAppReturnType::Type Force = FMessageDialog::Open(EAppMsgType::YesNo,
			FText::Format(LOCTEXT("PendingDraftErrors", "The draft contains {0} error(s):\n\n{1}\n\nYes: submit anyway.\nNo: discard the changes."),
				FText::AsNumber(Draft->GetValidation().CountErrors()), Draft->GetErrorSummary()));
		if (Force == EAppReturnType::Yes)
		{
			SubmitDraft(/*bIgnoreErrors*/ true);
		}
	}

	// Dans tous les cas, plus rien n'est en attente
	RevertDraft();
}

#undef LOCTEXT_NAMESPACE
