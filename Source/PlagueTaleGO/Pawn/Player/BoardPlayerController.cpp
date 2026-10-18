// BoardPlayerController.cpp
#include "BoardPlayerController.h"
#include "APlayerPawn.h"
#include "../../Node/ANode.h"
#include "PlagueTaleGO/GameMode/StateMachineGameMode.h"
#include "Camera/CameraActor.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

void ABoardPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;

	TActorIterator<ACameraActor> CameraIt(GetWorld());
	if (CameraIt)
	{
		SetViewTarget(*CameraIt);
	}

	// Ajout du Mapping Context pour les touches de la state machine
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ABoardPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (PrimaryInputAction)
		{
			EnhancedInputComponent->BindAction(PrimaryInputAction, ETriggerEvent::Started, this, &ABoardPlayerController::PrimaryAction);
		}
		if (SecondaryInputAction)
		{
			EnhancedInputComponent->BindAction(SecondaryInputAction, ETriggerEvent::Started, this, &ABoardPlayerController::SecondaryAction);
		}
	}
}

void ABoardPlayerController::PrimaryAction()
{
	if (AStateMachineGameMode* GameMode = Cast<AStateMachineGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (UStateBase* ActualState = GameMode->GetActualState())
		{
			ActualState->PrimaryInput();
			GameMode->GoToNextState();
		}
	}
}

void ABoardPlayerController::SecondaryAction()
{
	if (AStateMachineGameMode* GameMode = Cast<AStateMachineGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (UStateBase* ActualState = GameMode->GetActualState())
		{
			ActualState->SecondaryInput();
			GameMode->GoToNextState();
		}
	}
}

void ABoardPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (!PlayerPawn)
	{
		TryBindPlayerPawn();
	}

	if (WasInputKeyJustPressed(EKeys::LeftMouseButton))
	{
		HandleClick();
	}
}

void ABoardPlayerController::TryBindPlayerPawn()
{
	TActorIterator<APlayerPawn> PlayerIt(GetWorld());
	if (PlayerIt)
	{
		PlayerPawn = *PlayerIt;
		PlayerPawn->OnMoveFinished.AddDynamic(this, &ABoardPlayerController::HandlePlayerMoveFinished);
		UE_LOG(LogTemp, Warning, TEXT("[Init] pion lié : %s, nœud=%s"),
			*GetNameSafe(PlayerPawn), *GetNameSafe(PlayerPawn->GetCurrentNode()));
	}
}

void ABoardPlayerController::HandleClick()
{
	AStateMachineGameMode* GameMode = Cast<AStateMachineGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		return;
	}
	if (!GameMode->CanPlayerAct())
	{
		return;
	}
	if (!PlayerPawn)
	{
		return;
	}

	FHitResult Hit;
	if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		return;
	}

	AANode* ClickedNode = Cast<AANode>(Hit.GetActor());
	if (!ClickedNode)
	{
		return;
	}

	const EMoveResult Result = PlayerPawn->RequestMove(ClickedNode);
	UE_LOG(LogTemp, Warning, TEXT("[Click] nœud choisi : %s"), *GetNameSafe(ClickedNode));
	UE_LOG(LogTemp, Warning, TEXT("Move result: %s"), *UEnum::GetValueAsString(Result));
}

void ABoardPlayerController::HandlePlayerMoveFinished(ABoardPawn* MovedPawn)
{
	AStateMachineGameMode* GameMode = Cast<AStateMachineGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->GoToNextState();
	}
}
