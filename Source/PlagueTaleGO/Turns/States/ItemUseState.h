// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Turns/GameFlowState.h"
#include "PlagueTaleGO/Items/BoardItem.h"
#include "ItemUseState.generated.h"

class ABoardNode;

/**
 * Début du tour joueur quand un objet est en attente : le joueur DOIT l'utiliser avant de bouger.
 * Objet directionnel : les cases sélectionnables passent en jaune ; cliquer l'une d'elles choisit sa direction.
 * Puis effet + feedback, et le chef d'orchestre enchaîne sur UPlayerMoveState dans le même tour.
 */
UCLASS()
class PLAGUETALEGO_API UItemUseState : public UGameFlowState
{
	GENERATED_BODY()

public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual bool HandleIntent(const FPlayerIntent& Intent) override;

	// Le joueur ne peut cliquer que pendant le choix
	virtual bool AllowsPlayerInput() const override { return Phase == EPhase::Choosing; }

private:
	enum class EPhase : uint8
	{
		Choosing,   // cases hightlight affichées, on attend le clic
		Feedback,   // effet appliqué, on attend la fin du feedback
		Done
	};

	// Remplit le plateau, le joueur et sa case. Faux s'il manque quelque chose.
	bool BuildContext(FBoardItemUseContext& OutContext) const;

	// Cases jaunes : afficher / enlever
	void ShowOptions();
	void HideOptions();

	// Applique l'objet sur les cases touchées, puis lance le feedback
	void ApplyToNodes(const TArray<ABoardNode*>& Nodes);

	// Feedback de résultat (tâche 17 : remplacé par un fondu)
	void StartFeedback(const TArray<ABoardNode*>& Nodes);
	void OnFeedbackFinished();

	// Fin de l'utilisation : l'objet est consommé, l'état signale qu'il a fini
	void EndUse();

	UPROPERTY()
	TObjectPtr<UBoardItem> Item;

	// Directions valides et leurs cases (calculées à l'entrée)
	UPROPERTY()
	TArray<FBoardDirectionOption> Options;

	// Cases allumées par le feedback, à éteindre à la fin
	UPROPERTY()
	TArray<TObjectPtr<ABoardNode>> LitNodes;

	FTimerHandle FeedbackTimer;
	EPhase Phase = EPhase::Choosing;
};
