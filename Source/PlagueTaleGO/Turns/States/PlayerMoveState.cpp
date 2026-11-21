// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerMoveState.h"

#include "PlagueTaleGO/PlagueTaleGO.h"
#include "PlagueTaleGO/Entities/Player/APlayerPawn.h"
#include "PlagueTaleGO/Turns/GameFlowSubsystem.h"

void UPlayerMoveState::Enter()
{
	if (APlayerPawn* Player = GetOwner()->GetPlayerPawn())
	{
		Player->OnMoveFinished.AddDynamic(this, &UPlayerMoveState::HandleMoveFinished);
	}
}

void UPlayerMoveState::Exit()
{
	// Toujours se désabonner : l'ancien état ne doit plus recevoir d'événements
	if (APlayerPawn* Player = GetOwner()->GetPlayerPawn())
	{
		Player->OnMoveFinished.RemoveDynamic(this, &UPlayerMoveState::HandleMoveFinished);
	}
}

bool UPlayerMoveState::HandleIntent(const FPlayerIntent& Intent)
{
	if (bMoveInProgress || Intent.Type != EPlayerIntentType::SelectNode)
	{
		return false;
	}

	APlayerPawn* Player = GetOwner()->GetPlayerPawn();
	if (!Player)
	{
		return false;
	}

	const EMoveResult Result = Player->RequestMove(Intent.TargetNode);
	UE_LOG(LogPlagueTaleGO, Verbose, TEXT("[PlayerMove] %s -> %s"),
		*GetNameSafe(Intent.TargetNode), *UEnum::GetValueAsString(Result));

	if (Result != EMoveResult::Accepted)
	{
		return false;   // case non voisine, occupée, bloquée…
	}

	bMoveInProgress = true;   // on attend la fin de l'animation
	return true;
}

void UPlayerMoveState::HandleMoveFinished(ABoardPawn* MovedPawn)
{
	if (bMoveInProgress)
	{
		Finish();   // le chef d'orchestre décide de la suite (Victory ou EnemyTurn)
	}
}