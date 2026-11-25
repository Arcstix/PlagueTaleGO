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
#include "PlagueTaleGO/Items/BoardItem.h"
#include "PlagueTaleGO/Board/BoardNode.h"
#include "PlagueTaleGO/Level/Definitions/BoardItemType.h"
#include "PlagueTaleGO/Audio/AudioTags.h"
#include "PlagueTaleGO/Audio/BoardAudioSubsystem.h"
// Include complet : convertir UBoardNodeType* en UBoardDefinition* (contexte audio) exige la classe complète
#include "PlagueTaleGO/Level/Definitions/BoardNodeType.h"

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

void UGameFlowSubsystem::SubmitHover(ABoardNode* Node)
{
	if (CurrentState && CanPlayerAct())
	{
		CurrentState->HandleHover(Node);
		PlayHoverSound(Node);
	}
}

void UGameFlowSubsystem::PlayHoverSound(ABoardNode* Node) const
{
	// Case quittée (Node nul) ou case où le joueur ne peut rien faire : silence.
	// IsNodeSelectable de l'état courant : voisines reliées pendant le déplacement, cases jaunes pendant le choix d'un objet.
	if (!Node || !CurrentState || !CurrentState->IsNodeSelectable(Node))
	{
		return;
	}
	UBoardAudioSubsystem* Audio = UBoardAudioSubsystem::Get(this);
	if (!Audio)
	{
		return;
	}

	// Case qui donne un objet : son propre au type d'objet. Sinon : son de la case, propre à son type.
	const UBoardSubsystem* Board = GetBoard();
	if (UBoardItemType* ItemType = Board ? Board->GetItemTypeAt(Node) : nullptr)
	{
		Audio->PlaySound2D(AudioTags::UI_HoverItem, ItemType);
	}
	if (Node->GetNodeType())
	{
		Audio->PlaySound2D(AudioTags::UI_HoverNode, Node->GetNodeType());
	}
}

bool UGameFlowSubsystem::IsNodeSelectable(ABoardNode* Node) const
{
	return CurrentState && CanPlayerAct() && CurrentState->IsNodeSelectable(Node);
}

bool UGameFlowSubsystem::CanPlayerAct() const
{
	return CurrentState && CurrentState->AllowsPlayerInput();
}

UBoardItem* UGameFlowSubsystem::ConsumePendingItem()
{
	UBoardItem* Item = PendingItem;
	PendingItem = nullptr;
	return Item;
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

// ---------------------------------------------------------------------------
// Conséquences des états
// ---------------------------------------------------------------------------
void UGameFlowSubsystem::HandleFinishedState()
{
	if (!CurrentState)
	{
		return;   // début de partie : aucun état n'a encore fini
	}

	if (CurrentState->IsA<UPlayerMoveState>())
	{
		TryPickUpItem();
	}
}

void UGameFlowSubsystem::TryPickUpItem()
{
	UBoardSubsystem* Board = GetBoard();
	ABoardNode* Node = PlayerPawn ? PlayerPawn->GetCurrentNode() : nullptr;
	if (!Board || !Node)
	{
		return;
	}

	// La victoire reste prioritaire : sur une sortie, on gagne sans prendre l'objet
	if (Board->IsExit(Node) || !Board->HasItemAt(Node))
	{
		return;
	}

	// Un seul objet en attente : il est toujours utilisé au tour suivant, donc ce cas ne devrait pas arriver
	if (PendingItem)
	{
		UE_LOG(LogPlagueTaleGO, Warning, TEXT("[GameFlow] Un objet est déjà en attente : %s n'est pas pris."),
			*GetNameSafe(Node));
		return;
	}

	UBoardItemType* Type = Board->ConsumeItemAt(Node);
	if (!Type || !Type->ItemClass)
	{
		UE_LOG(LogPlagueTaleGO, Error, TEXT("[GameFlow] Objet sans ItemClass sur %s : rien n'est pris."), *GetNameSafe(Node));
		return;
	}

	// L'objet appartient au chef d'orchestre jusqu'à son utilisation
	PendingItem = NewObject<UBoardItem>(this, Type->ItemClass);
	PendingItem->SetItemType(Type);

	UE_LOG(LogPlagueTaleGO, Log, TEXT("[GameFlow] Objet pris sur %s : %s"),
		*GetNameSafe(Node), *GetNameSafe(PendingItem));
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

		HandleFinishedState();

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
