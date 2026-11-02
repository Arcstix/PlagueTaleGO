// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Turns/GameFlowState.h"
#include "StartState.generated.h"

// Premier état de la partie. Finit tout de suite pour l'instant
// (plus tard : intro, caméra, affichage du niveau…).
UCLASS()
class PLAGUETALEGO_API UStartState : public UGameFlowState
{
	GENERATED_BODY()

public:
	virtual void Enter() override;
};
