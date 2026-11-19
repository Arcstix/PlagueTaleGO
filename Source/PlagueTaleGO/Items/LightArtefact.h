// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BoardItem.h"
#include "PlagueTaleGO/Board/BoardHighlightType.h"
#include "LightArtefact.generated.h"

/**
 * Artefact de lumière : éclaire les cases dans une direction choisie par le joueur (nord, sud, est, ouest),
 * jusqu'à une certaine distance. Le choix ne dépend pas des liens : on peut éclairer
 * vers une case qui n'est pas reliée à celle du joueur.
 * Contexte (pas encore implémenté) : les ennemis qui se trouvent dans la zone éclairée mourront.
 * Les valeurs (portée…) se règlent dans le Blueprint enfant BP_LightArtefact.
 */
UCLASS()
class PLAGUETALEGO_API ULightArtefact : public UBoardItem
{
	GENERATED_BODY()

public:
	ULightArtefact();

	// --- Utilisation ---

	// Cases éclairées dans Context.Direction
	virtual TArray<ABoardNode*> Use(const FBoardItemUseContext& Context) override;

	// --- Réglages ---

	UFUNCTION(BlueprintPure, Category = "Item")
	int32 GetRange() const { return Range; }

	// Feedback : fondu de lumière sur les cases éclairées
	virtual float PlayFeedback(const FBoardItemUseContext& Context, const TArray<ABoardNode*>& AffectedNodes) override;
	virtual void StopFeedback(const TArray<ABoardNode*>& AffectedNodes) override;

protected:
	// Nombre de cases éclairées à partir de la case du joueur (non comprise)
	UPROPERTY(EditDefaultsOnly, Category = "Item", meta = (ClampMin = "1",
		DisplayName = "Range (cells)",
		ToolTip = "Number of grid positions lit in the chosen direction, starting next to the player. A position without a cell still counts."))
	int32 Range = 3;

	// Vrai : la lumière s'arrête à la première coordonnée sans case.
	// Faux : elle saute les trous et continue jusqu'à la portée. En attente de la décision des GD.
	UPROPERTY(EditDefaultsOnly, Category = "Item", meta = (
		DisplayName = "Stop At Gap",
		ToolTip = "If checked, the light stops at the first grid position without a cell. Otherwise it skips it (the gap still uses up range)."))
	bool bStopAtGap = false;

	// Fondu joué sur les cases éclairées
	UPROPERTY(EditDefaultsOnly, Category = "Item|Feedback", meta = (
		ToolTip = "Light fade played on the lit cells once the artefact is used."))
	FBoardHighlightFade LightFade;
};
