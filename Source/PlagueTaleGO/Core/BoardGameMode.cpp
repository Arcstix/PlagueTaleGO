// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardGameMode.h"
#include "PlagueTaleGO/Turns/StateBase.h"
#include "PlagueTaleGO/Entities/ABoardPawn.h"
#include "PlagueTaleGO/Board/BoardNode.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "PlagueTaleGO/PlagueTaleGO.h"
#include "PlagueTaleGO/Entities/Player/APlayerPawn.h"

void ABoardGameMode::StartPlay()
{
	// Super::StartPlay déclenche le BeginPlay de tous les acteurs du niveau.
	// Avant lui, le moteur a déjà appelé UBoardSubsystem::OnWorldBeginPlay :
	// le graphe est donc construit quand on fait apparaître les pions.
	Super::StartPlay();

	SpawnBoardEntities();
}

void ABoardGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	StartStateMachineOnStart();
	
}

void ABoardGameMode::SpawnBoardEntities()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	PlayerPawn = nullptr;

	for (TActorIterator<ABoardNode> It(World); It; ++It)
	{
		ABoardNode* Node = *It;
		const TSubclassOf<ABoardPawn> EntityClass = Node->GetStartingEntity();
		if (!EntityClass)
		{
			continue;   // case sans pion de départ
		}

		// AlwaysSpawn : ne pas annuler l'apparition si le mesh de la case gêne
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ABoardPawn* Pawn = World->SpawnActor<ABoardPawn>(
			EntityClass, Node->GetActorLocation(), Node->GetActorRotation(), Params);

		if (!Pawn)
		{
			UE_LOG(LogPlagueTaleGO, Error, TEXT("[GameMode] Impossible de créer %s sur %s (classe abstraite ?)"),
				*GetNameSafe(EntityClass), *GetNameSafe(Node));
			continue;
		}

		// Place le pion et l'enregistre dans UBoardSubsystem
		Pawn->SnapToNode(Node);

		// Le pion joueur est celui dont la classe dérive de APlayerPawn
		if (APlayerPawn* AsPlayer = Cast<APlayerPawn>(Pawn))
		{
			if (PlayerPawn)
			{
				UE_LOG(LogPlagueTaleGO, Error, TEXT("[GameMode] Plusieurs départs joueur : %s ignoré comme joueur."),
					*GetNameSafe(Node));
			}
			else
			{
				PlayerPawn = AsPlayer;
			}
		}

		UE_LOG(LogPlagueTaleGO, Log, TEXT("[GameMode] %s créé sur %s (%d, %d)"),
			*GetNameSafe(Pawn), *GetNameSafe(Node), Node->GetGridCoord().X, Node->GetGridCoord().Y);
	}

	if (!PlayerPawn)
	{
		UE_LOG(LogPlagueTaleGO, Error, TEXT("[GameMode] Aucun départ joueur dans le niveau."));
	}
}

void ABoardGameMode::StartStateMachineOnStart()
{
	if (FirstState)
	{
		CurrentState = NewObject<UStateBase>(this, FirstState);
		DebugLogActualState();
		CurrentState->EnterState();
		
	}
}

void ABoardGameMode::DebugLogActualState()
{
	if (!CurrentState) return;
	
	UE_LOG(LogPlagueTaleGO, Log, TEXT("État courant : %s"), *CurrentState->GetName());

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
			CurrentState->GetName());
	}
#endif
}

bool ABoardGameMode::CanPlayerAct() const
{
	return CurrentState && CurrentState->AllowsPlayerInput();
}

void ABoardGameMode::GoToNextState()
{
	if (CurrentState)
	{
		TSubclassOf<UStateBase> nextState = CurrentState->GetNextState();
		if (nextState)
		{
			CurrentState->ExitState();
			CurrentState = NewObject<UStateBase>(this, nextState);
			CurrentState->EnterState();
		
			DebugLogActualState();
		}
		
		
	}
}
