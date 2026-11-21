// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlagueTaleGO/Camera/BoardCameraTypes.h"
#include "BoardCameraRig.generated.h"

class USceneComponent;
class USpringArmComponent;
class UCameraComponent;
class UBoardCameraBehavior;
class ABoardNode;

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

	// Pose de cadrage : pivot = position du rig, angle et distance de départ.
	// Les comportements gardent le pivot et appliquent leur propre point de vue.
	FBoardCameraPose GetFramingPose() const;

	// --- Entrées, transmises au comportement (appelées par le controller) ---
	void HandleViewStep(int32 Step);
	void HandleDragStart();
	void HandleDrag(const FVector2D& MouseDelta);
	void HandleDragEnd();
	void HandlePlayerArrived(ABoardNode* Node);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

	// Fonctionnement de la caméra : choisi dans la liste déroulante, ses réglages s'affichent dessous
	UPROPERTY(EditAnywhere, Instanced, Category = "Camera|Behavior", meta = (
		ToolTip = "How the camera reacts to the player's inputs. Pick a behavior, its settings appear below. Empty = start pose, no input handled."))
	TObjectPtr<UBoardCameraBehavior> Behavior;
	
	// --- Pose de départ (le pivot est la position de l'acteur) ---
	// Remplacée plus tard par le comportement (tâche 23) et le cadrage automatique (tâche 24)

	UPROPERTY(EditAnywhere, Category = "Camera|Start", meta = (
		ClampMin = "-89.9", ClampMax = "89.9", Units = "Degrees",
		ToolTip = "Vertical angle used when no behavior is set. The pivot is the actor location."))
	float StartPitch = -45.f;

	UPROPERTY(EditAnywhere, Category = "Camera|Start", meta = (
		Units = "Degrees", ToolTip = "Horizontal angle at start if no behavior set."))
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
