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

void ABoardPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	/* Bind Actions */
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (SelectAction)
		{
			EnhancedInput->BindAction(SelectAction, ETriggerEvent::Started, this, &ABoardPlayerController::HandleSelect);
		}
	}
}

void ABoardPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	UpdateHover();
}

void ABoardPlayerController::HandleSelect()
{
	UGameFlowSubsystem* GameFlow = GetWorld()->GetSubsystem<UGameFlowSubsystem>();
	if (!GameFlow || !GameFlow->CanPlayerAct())
	{
		return;   // ce n'est pas au joueur de jouer (animation, tour ennemi, fin de partie…)
	}

	ABoardNode* Node = GetNodeUnderCursor();
	if (!Node)
	{
		return;   // clic dans le vide ou sur le décor
	}

	FPlayerIntent Intent;
	Intent.Type = EPlayerIntentType::SelectNode;
	Intent.TargetNode = Node;

	const bool bAccepted = GameFlow->SubmitPlayerIntent(Intent);
	UE_LOG(LogPlagueTaleGO, Verbose, TEXT("[Controller] %s sélectionnée : %s"),
		*GetNameSafe(Node), bAccepted ? TEXT("acceptée") : TEXT("refusée"));
	// Plus tard : son ou effet visuel si refusée
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
