// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlagueTaleGO/Camera/BoardCameraTypes.h"
#include "BoardCameraRig.generated.h"

class USceneComponent;
class USpringArmComponent;
class UCameraComponent;

/**
 * Caméra du plateau : un bras et une caméra au bout.
 * Le rig garde une pose cible et une pose actuelle ; l'actuelle rejoint la cible à chaque image.
 * Il ne décide jamais de la cible lui-même : ce sera le rôle du comportement (tâche 23).
 */
UCLASS()
class PLAGUETALEGO_API ABoardCameraRig : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABoardCameraRig();
	virtual void Tick(float DeltaTime) override;

	// Nouvelle cible : la caméra la rejoint en douceur, ou immédiatement si bInstant
	void SetTargetPose(const FBoardCameraPose& NewTarget, bool bInstant = false);

	const FBoardCameraPose& GetTargetPose() const { return TargetPose; }
	const FBoardCameraPose& GetCurrentPose() const { return CurrentPose; }

	// Vrai tant que la caméra n'a pas atteint sa cible
	bool IsMoving() const { return IsActorTickEnabled(); }

	// Test temporaire du lissage (à retirer à la tâche 25) : bascule entre la vue de départ et une vue de dessus
	UFUNCTION(CallInEditor, Category = "Camera|Debug")
	void DebugToggleTopView();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

	// --- Pose de départ (le pivot est la position de l'acteur) ---
	// Remplacée plus tard par le comportement (tâche 23) et le cadrage automatique (tâche 24)

	UPROPERTY(EditAnywhere, Category = "Camera|Start", meta = (
		ClampMin = "-89.9", ClampMax = "89.9", Units = "Degrees",
		ToolTip = "Vertical angle at start. The pivot is the actor location."))
	float StartPitch = -45.f;

	UPROPERTY(EditAnywhere, Category = "Camera|Start", meta = (
		Units = "Degrees", ToolTip = "Horizontal angle at start."))
	float StartYaw = 0.f;

	UPROPERTY(EditAnywhere, Category = "Camera|Start", meta = (
		ClampMin = "0", Units = "Centimeters", ToolTip = "Distance between the pivot and the camera at start."))
	float StartArmLength = 1500.f;

	// --- Lissage (vitesses d'interpolation, 0 = instantané) ---

	UPROPERTY(EditAnywhere, Category = "Camera|Smoothing", meta = (
		ClampMin = "0", ToolTip = "How fast the pivot reaches its target. 0 = instant."))
	float PivotSpeed = 6.f;

	UPROPERTY(EditAnywhere, Category = "Camera|Smoothing", meta = (
		ClampMin = "0", ToolTip = "How fast the rotation reaches its target. 0 = instant."))
	float RotationSpeed = 8.f;

	UPROPERTY(EditAnywhere, Category = "Camera|Smoothing", meta = (
		ClampMin = "0", ToolTip = "How fast the arm length reaches its target. 0 = instant."))
	float ArmLengthSpeed = 6.f;

private:
	FBoardCameraPose MakeStartPose() const;

	// Place l'acteur sur le pivot et oriente le bras
	void ApplyPose(const FBoardCameraPose& Pose);

	// Oriente le bras sans déplacer l'acteur (utilisable dans OnConstruction)
	void ApplyArm(const FBoardCameraPose& Pose);

	bool HasReachedTarget() const;

	// Visibles dans le Details du rig en PIE, pour le débogage
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Camera|Debug")
	FBoardCameraPose TargetPose;

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Camera|Debug")
	FBoardCameraPose CurrentPose;
};
