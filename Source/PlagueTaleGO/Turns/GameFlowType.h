// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Board/BoardNode.h"
#include "GameFlowType.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EPlayerIntentType : uint8
{
	SelectNode,   // clic sur une case : chaque état l'interprète (aller, viser…)
	UseItem       // utiliser l'objet sans cible (bouton d'interface, plus tard)
};

USTRUCT(BlueprintType)
struct FPlayerIntent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite) EPlayerIntentType Type = EPlayerIntentType::SelectNode;
	UPROPERTY(BlueprintReadWrite) TObjectPtr<ABoardNode> TargetNode = nullptr;
};