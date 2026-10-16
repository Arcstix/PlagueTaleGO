// BoardPlayerController.cpp
#include "BoardPlayerController.h"
#include "APlayerPawn.h"
#include "../../Node/ANode.h"
#include "PlagueTaleGO/GameMode/StateMachineGameMode.h"
#include "Camera/CameraActor.h"
#include "EngineUtils.h"
#include "Engine/World.h"

void ABoardPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;

	// Première caméra trouvée dans le niveau
	TActorIterator<ACameraActor> CameraIt(GetWorld());
	if (CameraIt)
	{
		SetViewTarget(*CameraIt);
	}

	// Premier pion joueur trouvé dans le niveau
	TActorIterator<APlayerPawn> PlayerIt(GetWorld());
	if (PlayerIt)
	{
		PlayerPawn = *PlayerIt;
		PlayerPawn->OnMoveFinished.AddDynamic(this, &ABoardPlayerController::HandlePlayerMoveFinished);
	}
}

void ABoardPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (WasInputKeyJustPressed(EKeys::LeftMouseButton))
	{
		HandleClick();
	}
}

void ABoardPlayerController::HandleClick()
{
	AStateMachineGameMode* GameMode = Cast<AStateMachineGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode || !GameMode->CanPlayerAct() || !PlayerPawn)
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
	UE_LOG(LogTemp, Log, TEXT("Move result: %s"), *UEnum::GetValueAsString(Result));
}

void ABoardPlayerController::HandlePlayerMoveFinished(ABoardPawn* MovedPawn)
{
	AStateMachineGameMode* GameMode = Cast<AStateMachineGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->GoToNextState();
	}
}
