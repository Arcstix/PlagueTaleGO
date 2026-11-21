// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Level/Definitions/BoardDefinition.h"
#include "PlagueTaleGO/Camera/BoardCameraTypes.h"
#include "BoardCameraView.generated.h"

/**
 * Un point de vue de la caméra : angles et distance autour du pivot du rig.
 * Utilisé par les comportements (vue par défaut) et par les cases du Board Editor.
 * Le pivot n'en fait pas partie : c'est toujours la position du rig.
 */
UCLASS(BlueprintType, meta = (ToolTip = "A camera view for the Board Editor: angles and distance around the camera rig."))
class PLAGUETALEGO_API UBoardCameraView : public UBoardDefinition
{
	GENERATED_BODY()
public:
	// Remplace les angles et la distance de la pose ; garde son pivot
	void ApplyTo(FBoardCameraPose& Pose) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera View", meta = (
		ClampMin = "-89.9", ClampMax = "89.9", Units = "Degrees",
		ToolTip = "Vertical angle. -90 looks straight down, 0 is parallel to the ground."))
	float Pitch = -45.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera View", meta = (
		Units = "Degrees",
		ToolTip = "Horizontal angle around the rig."))
	float Yaw = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera View", meta = (
		ClampMin = "0", Units = "Centimeters",
		ToolTip = "Distance between the rig and the camera."))
	float ArmLength = 1500.f;
	
};
