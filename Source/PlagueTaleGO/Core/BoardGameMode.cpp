// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardGameMode.h"
#include "PlagueTaleGO/Entities/ABoardPawn.h"
#include "PlagueTaleGO/Board/Board.h"
#include "PlagueTaleGO/Board/BoardNode.h"
#include "PlagueTaleGO/Board/BoardSubsystem.h"
#include "PlagueTaleGO/Level/BoardLayout.h"
#include "PlagueTaleGO/Level/BoardPlacementSettings.h"
#include "PlagueTaleGO/Level/Definitions/BoardEntityType.h"
#include "Engine/Engine.h"
#include "PlagueTaleGO/PlagueTaleGO.h"
#include "PlagueTaleGO/Entities/Player/APlayerPawn.h"
#include "PlagueTaleGO/Turns/GameFlowSubsystem.h"

void ABoardGameMode::StartPlay()
{
	// Super::StartPlay déclenche le BeginPlay de tous les acteurs du niveau.
	// Avant lui, le moteur a déjà appelé UBoardSubsystem::OnWorldBeginPlay :
	// le graphe est donc construit quand on fait apparaître les pions.
	Super::StartPlay();

	SpawnBoardEntities();

	// 3. Lance la machine à états : StartState → PlayerMoveState…
	UGameFlowSubsystem* GameFlow = GetWorld()->GetSubsystem<UGameFlowSubsystem>();
	if (!GameFlow)
	{
		UE_LOG(LogPlagueTaleGO, Error, TEXT("[GameMode] UGameFlowSubsystem introuvable : partie non lancée."));
		return;
	}

	GameFlow->StartGame(PlayerPawn);
}

void ABoardGameMode::SpawnBoardEntities()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	PlayerPawn = nullptr;

	UBoardSubsystem* BoardSubsystem = World->GetSubsystem<UBoardSubsystem>();
	const ABoard* Board = BoardSubsystem ? BoardSubsystem->GetBoard() : nullptr;
	if (!Board)
	{
		UE_LOG(LogPlagueTaleGO, Error, TEXT("[GameMode] Aucun ABoard : aucun pion créé."));
		return;
	}

	// Les pions de départ sont décrits par le layout du Board (écrit par le Level Editor)
	for (const FBoardCell& Cell : Board->GetLayout().Cells)
	{
		for (const FBoardEntityPlacement& Placement : Cell.Entities)
		{
			SpawnEntity(Cell, Placement, *BoardSubsystem);
		}
	}

	if (!PlayerPawn)
	{
		UE_LOG(LogPlagueTaleGO, Error, TEXT("[GameMode] Aucun départ joueur dans le niveau."));
	}
}

void ABoardGameMode::SpawnEntity(const FBoardCell& Cell, const FBoardEntityPlacement& Placement, UBoardSubsystem& BoardSubsystem)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const UBoardEntityType* Type = Placement.Type;
	if (!Type || !Type->PawnClass)
	{
		UE_LOG(LogPlagueTaleGO, Error, TEXT("[GameMode] Entité sans type (ou type sans PawnClass) sur (%d, %d) : ignorée."),
			Cell.Coord.X, Cell.Coord.Y);
		return;
	}

	ABoardNode* Node = BoardSubsystem.GetNodeAt(Cell.Coord);
	if (!Node)
	{
		UE_LOG(LogPlagueTaleGO, Error, TEXT("[GameMode] Pas d'acteur pour la case (%d, %d) : %s ignorée (niveau non synchronisé)."),
			Cell.Coord.X, Cell.Coord.Y, *GetNameSafe(Type));
		return;
	}

	const FTransform SpawnTransform(
		FRotator(0.f, BoardDirection::ToYaw(Placement.Facing), 0.f),
		Node->GetActorLocation());

	// Création « différée » : l'acteur existe mais son BeginPlay n'est pas encore appelé.
	// On peut ainsi lui transmettre ses réglages AVANT BeginPlay, puis FinishSpawning termine la création.
	// AlwaysSpawn : ne pas annuler l'apparition si le mesh de la case gêne.
	ABoardPawn* Pawn = World->SpawnActorDeferred<ABoardPawn>(
		Type->PawnClass, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Pawn)
	{
		UE_LOG(LogPlagueTaleGO, Error, TEXT("[GameMode] Impossible de créer %s sur %s (classe abstraite ?)"),
			*GetNameSafe(Type->PawnClass), *GetNameSafe(Node));
		return;
	}

	Pawn->ApplyPlacementSettings(Placement.Settings);
	Pawn->FinishSpawning(SpawnTransform);

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
		*GetNameSafe(Pawn), *GetNameSafe(Node), Cell.Coord.X, Cell.Coord.Y);
}
