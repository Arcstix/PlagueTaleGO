// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFlowSubsystem.h"
#include "GameFlowState.h"
#include "PlagueTaleGO/PlagueTaleGO.h"
#include "PlagueTaleGO/Board/BoardSubsystem.h"
#include "PlagueTaleGO/Entities/Player/APlayerPawn.h"
#include "States/DefeatState.h"
#include "States/EnemyTurnState.h"
#include "States/ItemUseState.h"
#include "States/PlayerMoveState.h"
#include "States/StartState.h"
#include "States/VictoryState.h"

// ---------------------------------------------------------------------------
// Cycle de vie
// ---------------------------------------------------------------------------
bool UGameFlowSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UGameFlowSubsystem::Deinitialize()
{
	if (CurrentState)
	{
		CurrentState->Exit();
		CurrentState = nullptr;
	}
	Super::Deinitialize();
}

void UGameFlowSubsystem::StartGame(APlayerPawn* InPlayerPawn)
{
	if (CurrentState)
	{
		UE_LOG(LogPlagueTaleGO, Warning, TEXT("[GameFlow] StartGame appelé alors qu'une partie est en cours."));
		return;
	}

	PlayerPawn = InPlayerPawn;
	PendingItem = nullptr;

	if (!PlayerPawn)
	{
		UE_LOG(LogPlagueTaleGO, Error, TEXT("[GameFlow] StartGame sans pion joueur : partie non lancée."));
		return;
	}

	Advance();   // CurrentState est nul → DecideNextState renvoie UStartState
}

UBoardSubsystem* UGameFlowSubsystem::GetBoard() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetSubsystem<UBoardSubsystem>() : nullptr;
}

// ---------------------------------------------------------------------------
// Entrées : les états et le controller
// ---------------------------------------------------------------------------
void UGameFlowSubsystem::NotifyStateFinished(UGameFlowState* State)
{
	if (State != CurrentState)
	{
		return;   // signal d'un ancien état : ignoré
	}
	if (bIsChangingState)
	{
		bAdvanceRequested = true;   // sera traité à la fin du changement en cours
		return;
	}
	Advance();
}

bool UGameFlowSubsystem::SubmitPlayerIntent(const FPlayerIntent& Intent)
{
	return CurrentState && CurrentState->HandleIntent(Intent);
}

bool UGameFlowSubsystem::CanPlayerAct() const
{
	return CurrentState && CurrentState->AllowsPlayerInput();
}

// ---------------------------------------------------------------------------
// Les règles de transition
// ---------------------------------------------------------------------------
TSubclassOf<UGameFlowState> UGameFlowSubsystem::DecideNextState() const
{
	// Début de partie
	if (!CurrentState)
	{
		return UStartState::StaticClass();
	}

	if (CurrentState->IsA<UStartState>())
	{
		return StartOfPlayerTurn();
	}

	// L'objet a été utilisé : le joueur se déplace DANS LE MÊME TOUR
	if (CurrentState->IsA<UItemUseState>())
	{
		return UPlayerMoveState::StaticClass();
	}

	if (CurrentState->IsA<UPlayerMoveState>())
	{
		const UBoardSubsystem* Board = GetBoard();
		if (Board && PlayerPawn && Board->IsExit(PlayerPawn->GetCurrentNode()))
		{
			return UVictoryState::StaticClass();
		}
		// Ramassage de l'objet de la case ici
		return UEnemyTurnState::StaticClass();
	}

	if (CurrentState->IsA<UEnemyTurnState>())
	{
		if (IsPlayerCaught())
		{
			return UDefeatState::StaticClass();
		}
		return StartOfPlayerTurn();
	}

	// UVictoryState, UDefeatState : fin de partie, plus de transition
	return nullptr;
}

TSubclassOf<UGameFlowState> UGameFlowSubsystem::StartOfPlayerTurn() const
{
	// Objet en attente → il doit être utilisé avant de bouger
	return HasPendingItem()
		? TSubclassOf<UGameFlowState>(UItemUseState::StaticClass())
		: TSubclassOf<UGameFlowState>(UPlayerMoveState::StaticClass());
}

bool UGameFlowSubsystem::IsPlayerCaught() const
{
	return false;
}

// ---------------------------------------------------------------------------
// Changement d'état (privé : personne d'autre ne change d'état)
// ---------------------------------------------------------------------------
void UGameFlowSubsystem::Advance()
{
	do
	{
		bAdvanceRequested = false;

		const TSubclassOf<UGameFlowState> Next = DecideNextState();
		if (!Next)
		{
			UE_LOG(LogPlagueTaleGO, Log, TEXT("[GameFlow] Fin de partie (%s)."), *GetNameSafe(CurrentState));
			return;   // on reste dans Victory / Defeat
		}

		bIsChangingState = true;
		ChangeState(Next);
		bIsChangingState = false;
	}
	while (bAdvanceRequested);   // un état a fini pendant son Enter() → on enchaîne
}

void UGameFlowSubsystem::ChangeState(TSubclassOf<UGameFlowState> NextClass)
{
	if (CurrentState)
	{
		CurrentState->Exit();
	}

	CurrentState = NewObject<UGameFlowState>(this, NextClass);
	CurrentState->Initialize(this);

	UE_LOG(LogPlagueTaleGO, Log, TEXT("[GameFlow] -> %s"), *GetNameSafe(NextClass));
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, NextClass->GetName());
	}
#endif

	OnStateChanged.Broadcast(CurrentState);
	CurrentState->Enter();
}
