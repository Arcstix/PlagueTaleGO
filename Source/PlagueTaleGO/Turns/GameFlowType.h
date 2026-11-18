// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Board/BoardTypes.h"
#include "GameFlowType.generated.h"

class ABoardNode;

UENUM(BlueprintType)
enum class EPlayerIntentType : uint8
{
	SelectNode,   // clic sur une case : chaque état l'interprète (aller, viser…)
	SelectDirection,  // clic sur un indicateur de direction (objet directionnel)
	UseItem       // utiliser l'objet sans cible (bouton d'interface, plus tard)
};

/**
 * Une action du joueur, construite par le controller et transmise à l'état courant
 * via UGameFlowSubsystem::SubmitPlayerIntent. Seuls les champs utiles au Type sont remplis.
 */
USTRUCT(BlueprintType)
struct FPlayerIntent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Intent")
	EPlayerIntentType Type = EPlayerIntentType::SelectNode;

	// Case désignée (Type == SelectNode)
	UPROPERTY(BlueprintReadWrite, Category = "Intent")
	TObjectPtr<ABoardNode> TargetNode = nullptr;

	// Direction désignée (Type == SelectDirection)
	UPROPERTY(BlueprintReadWrite, Category = "Intent")
	EBoardDirection Direction = EBoardDirection::North;
};