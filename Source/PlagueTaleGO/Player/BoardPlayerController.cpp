// BoardPlayerController.cpp
#include "BoardPlayerController.h"
#include "PlagueTaleGO/Board/BoardNode.h"
#include "PlagueTaleGO/Core/BoardGameMode.h"
#include "Camera/CameraActor.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "PlagueTaleGO/PlagueTaleGO.h"
#include "PlagueTaleGO/Turns/GameFlowSubsystem.h"
#include "PlagueTaleGO/Camera/BoardCameraRig.h"
#include "InputActionValue.h"
#include "PlagueTaleGO/Board/BoardSubsystem.h"
#include "PlagueTaleGO/Entities/Player/APlayerPawn.h"

void ABoardPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;

	// Jeu à la souris : le curseur reste visible et ne se verrouille pas
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	/* Add Mapping Context */
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	SetupBoardCamera();
}

void ABoardPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ABoardPawn* BoardPlayer = BoundPlayerPawn.Get())
	{
		BoardPlayer->OnMoveFinished.RemoveDynamic(this, &ABoardPlayerController::HandlePlayerMoveFinished);
	}
	BoundPlayerPawn = nullptr;

	Super::EndPlay(EndPlayReason);
}

void ABoardPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	/* Bind Actions */
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (SelectAction)
		{
			EnhancedInput->BindAction(SelectAction, ETriggerEvent::Started, this, &ABoardPlayerController::HandleSelectStarted);
			EnhancedInput->BindAction(SelectAction, ETriggerEvent::Completed, this, &ABoardPlayerController::HandleSelectCompleted);
			EnhancedInput->BindAction(SelectAction, ETriggerEvent::Canceled, this, &ABoardPlayerController::HandleSelectCompleted);
		}
		if (CameraViewAction)
		{
			EnhancedInput->BindAction(CameraViewAction, ETriggerEvent::Triggered, this, &ABoardPlayerController::HandleCameraView);
		}
	}
}

void ABoardPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	TryBindPlayerPawn();
	UpdateHover();
	UpdateCameraDrag();
}

void ABoardPlayerController::HandleSelectStarted()
{
	UGameFlowSubsystem* GameFlow = GetWorld()->GetSubsystem<UGameFlowSubsystem>();
	ABoardNode* Node = GetNodeUnderCursor();
	
	// Case jouable : le clic sert au jeu, la caméra ne bouge pas
	if (GameFlow && Node && GameFlow->IsNodeSelectable(Node))
	{
		FPlayerIntent Intent;
		Intent.Type = EPlayerIntentType::SelectNode;
		Intent.TargetNode = Node;

		const bool bAccepted = GameFlow->SubmitPlayerIntent(Intent);
		UE_LOG(LogPlagueTaleGO, Verbose, TEXT("[Controller] %s sélectionnée : %s"),
			*GetNameSafe(Node), bAccepted ? TEXT("acceptée") : TEXT("refusée"));
		return;
	}
	
	// Partout ailleurs (décor, case non jouable, tour ennemi…) : glisser caméra
	if (CameraRig)
	{
		bCameraDragging = true;
		bHasLastCursorPosition = false;   // la position sera lue à la prochaine image
		CameraRig->HandleDragStart();
	}
}

void ABoardPlayerController::HandleSelectCompleted()
{
	if (!bCameraDragging)
	{
		return;
	}

	bCameraDragging = false;
	if (CameraRig)
	{
		CameraRig->HandleDragEnd();
	}
}

void ABoardPlayerController::TryBindPlayerPawn()
{
	if (BoundPlayerPawn.IsValid())
	{
		return;   // déjà abonné
	}

	UGameFlowSubsystem* GameFlow = GetWorld()->GetSubsystem<UGameFlowSubsystem>();
	APlayerPawn* PlayerPawn = GameFlow ? GameFlow->GetPlayerPawn() : nullptr;
	if (!PlayerPawn)
	{
		return;   // partie pas encore lancée : on réessaiera à la prochaine image
	}

	PlayerPawn->OnMoveFinished.AddDynamic(this, &ABoardPlayerController::HandlePlayerMoveFinished);
	BoundPlayerPawn = PlayerPawn;
}

void ABoardPlayerController::HandlePlayerMoveFinished(ABoardPawn* MovedPawn)
{
	if (!CameraRig)
	{
		return;
	}

	UBoardSubsystem* Board = GetWorld()->GetSubsystem<UBoardSubsystem>();
	ABoardNode* Node = Board ? Board->GetPawnNode(MovedPawn) : nullptr;
	CameraRig->HandlePlayerArrived(Node);
}

void ABoardPlayerController::UpdateCameraDrag()
{
	if (!bCameraDragging || !CameraRig)
	{
		return;
	}

	float CursorX = 0.f;
	float CursorY = 0.f;
	if (!GetMousePosition(CursorX, CursorY))
	{
		bHasLastCursorPosition = false;   // curseur indisponible : on repartira de sa prochaine position, sans saut
		return;
	}

	const FVector2D CursorPosition(CursorX, CursorY);
	if (!bHasLastCursorPosition)
	{
		LastCursorPosition = CursorPosition;   // première position connue : pas encore de mouvement
		bHasLastCursorPosition = true;
		return;
	}
	
	const FVector2D Delta = CursorPosition - LastCursorPosition;
	LastCursorPosition = CursorPosition;

	if (Delta.IsNearlyZero())
	{
		return;
	}
	
	// Écran : Y vers le bas. On l'inverse pour garder la convention « vers le haut = positif »
	CameraRig->HandleDrag(FVector2D(Delta.X, -Delta.Y));
}

void ABoardPlayerController::SetupBoardCamera()
{
	// N'importe quel acteur avec le tag : le rig, ou un ancien CameraActor
	TArray<AActor*> Cameras;
	UGameplayStatics::GetAllActorsWithTag(this, BoardCameraTag, Cameras);

	if (Cameras.Num() == 0)
	{
		UE_LOG(LogPlagueTaleGO, Warning, TEXT("[Controller] Aucun acteur avec le tag '%s' dans le niveau."),
			*BoardCameraTag.ToString());
		return;
	}
	if (Cameras.Num() > 1)
	{
		UE_LOG(LogPlagueTaleGO, Warning, TEXT("[Controller] Plusieurs acteurs avec le tag '%s' : %s utilisé."),
			*BoardCameraTag.ToString(), *GetNameSafe(Cameras[0]));
	}

	AActor* ViewActor = Cameras[0];
	CameraRig = Cast<ABoardCameraRig>(ViewActor);   // les tâches suivantes lui enverront les entrées
	SetViewTarget(ViewActor);
}

void ABoardPlayerController::UpdateHover()
{
	UWorld* World = GetWorld();
	UGameFlowSubsystem* GameFlow = World ? World->GetSubsystem<UGameFlowSubsystem>() : nullptr;
	if (!GameFlow || !GameFlow->CanPlayerAct())
	{
		// On oublie la case : quand le joueur reprend la main, la case sous le curseur sera renvoyée
		HoveredNode = nullptr;
		return;
	}

	ABoardNode* Node = GetNodeUnderCursor();
	if (Node == HoveredNode.Get())
	{
		return;   // on n'envoie que les changements, pas un appel par frame
	}

	HoveredNode = Node;
	GameFlow->SubmitHover(Node);
}

ABoardNode* ABoardPlayerController::GetNodeUnderCursor() const
{
	FHitResult Hit;
	if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		return nullptr;
	}
	return Cast<ABoardNode>(Hit.GetActor());
}

void ABoardPlayerController::HandleCameraView(const FInputActionValue& Value)
{
	// Pas de test CanPlayerAct : la caméra n'est pas de la logique de tour,
	// elle reste utilisable pendant les animations et le tour ennemi
	if (!CameraRig)
	{
		return;
	}

	const float Axis = Value.Get<float>();
	if (FMath::IsNearlyZero(Axis))
	{
		return;
	}

	CameraRig->HandleViewStep(Axis > 0.f ? 1 : -1);   // seul le sens compte, pas la vitesse de la molette
}
