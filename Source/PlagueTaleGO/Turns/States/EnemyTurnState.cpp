#include "EnemyTurnState.h"

#include "EngineUtils.h"
#include "PlagueTaleGO/PlagueTaleGO.h"
#include "PlagueTaleGO/Board/BoardNode.h"
#include "PlagueTaleGO/Board/BoardSubsystem.h"
#include "PlagueTaleGO/Entities/Player/APlayerPawn.h"
#include "PlagueTaleGO/Entities/Statue/AWormPawn.h"
#include "PlagueTaleGO/Turns/GameFlowSubsystem.h"

void UEnemyTurnState::Enter()
{
	Statue = FindWorm();
	if (!Statue)
	{
		UE_LOG(LogPlagueTaleGO, Log, TEXT("[EnemyTurn] Aucune statue : tour passé."));
		Finish();
		return;
	}

	ABoardNode* NextNode = FindNextStep(Statue);
	if (!NextNode || !MoveWorm(NextNode))
	{
		Finish();
		return;
	}

	// Sinon : on attend HandleStatueMoveFinished
}

void UEnemyTurnState::Exit()
{
	// Toujours se désabonner : l'ancien état ne doit plus recevoir d'événements
	if (Statue)
	{
		Statue->OnMoveFinished.RemoveDynamic(this, &UEnemyTurnState::HandleStatueMoveFinished);
	}
}

// ---------------------------------------------------------------------------
// 1. Trouver la statue
// ---------------------------------------------------------------------------
AWormPawn* UEnemyTurnState::FindWorm() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Parcourt tous les AStatuePawn du niveau : on s'arrête au premier
	for (TActorIterator<AWormPawn> It(World); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

// ---------------------------------------------------------------------------
// 2 + 3. Les deux cases, puis le chemin
// ---------------------------------------------------------------------------
ABoardNode* UEnemyTurnState::FindNextStep(const AWormPawn* InStatue) const
{
	const APlayerPawn* Player = GetOwner()->GetPlayerPawn();
	const UBoardSubsystem* Board = GetOwner()->GetBoard();
	if (!InStatue || !Player || !Board)
	{
		return nullptr;
	}

	ABoardNode* StatueNode = InStatue->GetCurrentNode();
	ABoardNode* PlayerNode = Player->GetCurrentNode();

	// true : contourne les cases occupées (sauf celle du joueur, l'arrivée), a rediscuter avec les GD
	const TArray<ABoardNode*> Path = Board->FindPath(StatueNode, PlayerNode, true);
	if (Path.IsEmpty())
	{
		UE_LOG(LogPlagueTaleGO, Log, TEXT("[EnemyTurn] Aucun chemin de %s vers le joueur."),
			*GetNameSafe(StatueNode));
		return nullptr;
	}

	const FIntPoint Coord = Path[0]->GetGridCoord();
	UE_LOG(LogPlagueTaleGO, Log, TEXT("[EnemyTurn] %s vise %s (%d, %d)"),
		*GetNameSafe(InStatue), *GetNameSafe(Path[0]), Coord.X, Coord.Y);

	return Path[0];
}

bool UEnemyTurnState::MoveWorm(ABoardNode* TargetNode)
{
	// On s'abonne AVANT de bouger : on ne peut pas rater la fin du mouvement
	Statue->OnMoveFinished.AddDynamic(this, &UEnemyTurnState::HandleStatueMoveFinished);

	const EMoveResult Result = Statue->RequestMove(TargetNode);
	if (Result != EMoveResult::Accepted)
	{
		UE_LOG(LogPlagueTaleGO, Log, TEXT("[EnemyTurn] Déplacement refusé : %s"),
			*UEnum::GetValueAsString(Result));
		Statue->OnMoveFinished.RemoveDynamic(this, &UEnemyTurnState::HandleStatueMoveFinished);
		return false;
	}
	return true;
}

void UEnemyTurnState::HandleStatueMoveFinished(ABoardPawn* MovedPawn)
{
	if (MovedPawn == Statue)
	{
		Finish();   
	}
}
