// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BoardCameraTypes.generated.h"

/**
 * Pose complète de la caméra du plateau.
 * C'est tout ce qu'un comportement peut demander au rig : le rig se charge de l'atteindre en douceur.
 */
USTRUCT(BlueprintType)
struct PLAGUETALEGO_API FBoardCameraPose
{
	GENERATED_BODY()

	// Point autour duquel la caméra tourne, et qu'elle regarde
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (
		ToolTip = "World point the camera turns around and looks at."))
	FVector Pivot = FVector::ZeroVector;

	// Angle vertical : -90 = face au sol, 0 = parallèle au sol
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (
		ClampMin = "-89.9", ClampMax = "-10", Units = "Degrees",
		ToolTip = "Vertical angle. -90 looks straight down, 0 is parallel to the ground."))
	float Pitch = -45.f;

	// Angle horizontal autour du pivot, non borné (370 = 10)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (
		Units = "Degrees",
		ToolTip = "Horizontal angle around the pivot. Not limited: 370 is the same as 10."))
	float Yaw = 0.f;

	// Distance entre le pivot et la caméra
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (
		ClampMin = "0", Units = "Centimeters",
		ToolTip = "Distance between the pivot and the camera."))
	float ArmLength = 1500.f;

	FRotator GetRotation() const { return FRotator(Pitch, Yaw, 0.f); }
};
