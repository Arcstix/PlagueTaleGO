// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BoardCameraBehavior.h"
#include "FixedCameraBehavior.generated.h"

class UBoardCameraView;

/**
 * Comportement le plus simple : une vue fixe, toutes les entrées sont ignorées.
 * Sert de test pour l'architecture, et de solution de secours.
 */
UCLASS(meta = (DisplayName = "Fixed"))
class PLAGUETALEGO_API UFixedCameraBehavior : public UBoardCameraBehavior
{
	GENERATED_BODY()

protected:
	virtual void OnActivated() override;

	// Point de vue affiché. Vide = pose de départ du rig.
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (
		ToolTip = "Camera view shown by this behavior. Empty = the rig's start pose."))
	TObjectPtr<UBoardCameraView> View;
};
