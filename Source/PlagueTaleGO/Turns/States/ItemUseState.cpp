// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemUseState.h"
#include "PlagueTaleGO/Turns/GameFlowSubsystem.h"
#include "PlagueTaleGO/Board/BoardNode.h"
#include "PlagueTaleGO/Board/BoardSubsystem.h"
#include "PlagueTaleGO/Entities/Player/APlayerPawn.h"
#include "PlagueTaleGO/PlagueTaleGO.h"
#include "Engine/World.h"
#include "TimerManager.h"

namespace
{
	// Cases sélectionnables pendant le choix
	const FLinearColor SelectableColor(1.f, 0.85f, 0.f);

	// Feedback provisoire du résultat (tâche 18 : couleur et durée réglables par objet)
	const FLinearColor ResultColor(1.f, 1.f, 0.9f);
	constexpr float FeedbackDuration = 0.8f;
}

// ---------------------------------------------------------------------------
// Cycle de vie
// ---------------------------------------------------------------------------
void UItemUseState::Enter()
{
	Phase = EPhase::Choosing;
	Item = GetOwner()->GetPendingItem();

	FBoardItemUseContext Context;
	if (!Item || !BuildContext(Context))
	{
		UE_LOG(LogPlagueTaleGO, Warning, TEXT("[ItemUse] Pas d'objet ou contexte incomplet : rien n'est utilisé."));
		EndUse();
		return;
	}

	switch (Item->GetTargeting())
	{
	case EBoardItemTargeting::Direction:
		Options = Item->GetDirectionOptions(Context);
		if (Options.Num() == 0)
		{
			// Aucune direction ne touche de case : objet consommé sans effet (à confirmer avec les GD)
			UE_LOG(LogPlagueTaleGO, Log, TEXT("[ItemUse] %s : aucune direction valide, objet consommé sans effet."),
				*GetNameSafe(Item));
			EndUse();
			return;
		}
		ShowOptions();   // on attend maintenant le clic du joueur (HandleIntent)
		break;

	case EBoardItemTargeting::None:
	default:
		// Objet sans choix : utilisé tout de suite
		ApplyToNodes(Context, Item->Use(Context));
		break;
	}
}

void UItemUseState::Exit()
{
	// Ce qui a été créé dans Enter est nettoyé ici, même si l'état est quitté plus tôt que prévu
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FeedbackTimer);
	}

	HideOptions();

	// Utilisation interrompue pendant le feedback : l'objet nettoie lui-même ce qu'il a affiché
	if (Item && AffectedNodes.Num() > 0)
	{
		TArray<ABoardNode*> Nodes;
		for (ABoardNode* Node : AffectedNodes)
		{
			Nodes.Add(Node);
		}
		Item->StopFeedback(Nodes);
	}
	AffectedNodes.Reset();
}

// ---------------------------------------------------------------------------
// Choix du joueur
// ---------------------------------------------------------------------------
bool UItemUseState::HandleIntent(const FPlayerIntent& Intent)
{
	if (Phase != EPhase::Choosing)
	{
		return false;
	}

	EBoardDirection Direction;
	if (Intent.Type == EPlayerIntentType::SelectNode)
	{
		// Clic sur une case : elle doit être une des cases jaunes
		if (!UBoardItem::FindDirectionOfNode(Options, Intent.TargetNode, Direction))
		{
			return false;   // clic ailleurs : ignoré
		}
	}
	else if (Intent.Type == EPlayerIntentType::SelectDirection)
	{
		// Prévu pour un futur contrôle clavier/manette : la direction doit être valide
		Direction = Intent.Direction;
		if (!Options.ContainsByPredicate([Direction](const FBoardDirectionOption& O) { return O.Direction == Direction; }))
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	FBoardItemUseContext Context;
	if (!BuildContext(Context))
	{
		return false;
	}
	Context.Direction = Direction;

	UE_LOG(LogPlagueTaleGO, Log, TEXT("[ItemUse] %s utilisé vers %s."),
		*GetNameSafe(Item), *UEnum::GetValueAsString(Direction));

	HideOptions();
	ApplyToNodes(Context,Item->Use(Context));
	return true;
}

void UItemUseState::HandleHover(ABoardNode* Node)
{
	if (Phase != EPhase::Choosing || !Item)
	{
		return;   // pendant le feedback, le survol ne change rien
	}

	// Une case n'appartient qu'à une seule direction : les branches de la croix ne se croisent pas
	int32 NewOption = INDEX_NONE;
	if (Node)
	{
		for (int32 Index = 0; Index < Options.Num(); ++Index)
		{
			if (Options[Index].Nodes.Contains(Node))
			{
				NewOption = Index;
				break;
			}
		}
	}

	if (NewOption == HoveredOption)
	{
		return;   // même direction : rien à refaire
	}

	// L'ancienne direction redevient jaune, la nouvelle s'illumine
	if (HoveredOption != INDEX_NONE)
	{
		SetOptionColor(HoveredOption, Item->GetSelectableColor());
	}
	HoveredOption = NewOption;
	if (HoveredOption != INDEX_NONE)
	{
		SetOptionColor(HoveredOption, Item->GetHoverColor());
	}
}

bool UItemUseState::BuildContext(FBoardItemUseContext& OutContext) const
{
	OutContext.Board = GetOwner()->GetBoard();
	OutContext.User = GetOwner()->GetPlayerPawn();
	OutContext.OriginNode = OutContext.User ? OutContext.User->GetCurrentNode() : nullptr;
	return OutContext.Board && OutContext.User && OutContext.OriginNode;
}

// ---------------------------------------------------------------------------
// Cases sélectionnables
// ---------------------------------------------------------------------------
void UItemUseState::ShowOptions()
{
	for (const FBoardDirectionOption& Option : Options)
	{
		for (ABoardNode* Node : Option.Nodes)
		{
			if (Node)
			{
				Node->SetHighlight(SelectableColor, 1.f);
			}
		}
	}
}

void UItemUseState::HideOptions()
{
	for (const FBoardDirectionOption& Option : Options)
	{
		for (ABoardNode* Node : Option.Nodes)
		{
			if (Node)
			{
				Node->ClearHighlight();
			}
		}
	}
	HoveredOption = INDEX_NONE;
}

void UItemUseState::SetOptionColor(int32 OptionIndex, const FLinearColor& Color)
{
	if (!Options.IsValidIndex(OptionIndex))
	{
		return;
	}
	for (ABoardNode* Node : Options[OptionIndex].Nodes)
	{
		if (Node)
		{
			Node->SetHighlight(Color, 1.f);
		}
	}
}

// ---------------------------------------------------------------------------
// Effet et feedback
// ---------------------------------------------------------------------------
void UItemUseState::ApplyToNodes(const FBoardItemUseContext& Context, const TArray<ABoardNode*>& Nodes)
{
	Phase = EPhase::Feedback;   // plus aucun clic accepté

	for (ABoardNode* Node : Nodes)
	{
		Item->ApplyToNode(Node);   // effet de jeu : vide pour l'instant
	}

	StartFeedback(Context, Nodes);
}

void UItemUseState::StartFeedback(const FBoardItemUseContext& Context, const TArray<ABoardNode*>& Nodes)
{
	AffectedNodes.Reset();
	for (ABoardNode* Node : Nodes)
	{
		AffectedNodes.Add(Node);
	}

	// L'objet joue SON feedback et indique combien de temps il dure
	const float Duration = Item->PlayFeedback(Context, Nodes);

	UWorld* World = GetWorld();
	if (!World || Duration <= 0.f)
	{
		OnFeedbackFinished();
		return;
	}
	World->GetTimerManager().SetTimer(FeedbackTimer, this, &UItemUseState::OnFeedbackFinished, Duration, false);
}

void UItemUseState::OnFeedbackFinished()
{
	AffectedNodes.Reset();   // feedback terminé normalement : rien à interrompre
	EndUse();
}

void UItemUseState::EndUse()
{
	if (Phase == EPhase::Done)
	{
		return;   // déjà fini : ne pas signaler deux fois
	}
	Phase = EPhase::Done;

	GetOwner()->ConsumePendingItem();
	Finish();   // le chef d'orchestre enchaîne sur UPlayerMoveState, dans le même tour
}
