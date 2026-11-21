// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PlagueTaleGO/Camera/BoardCameraTypes.h"
#include "BoardCameraBehavior.generated.h"

class ABoardCameraRig;
class ABoardNode;

/**
 * Fonctionnement de la caméra : reçoit les entrées et en déduit une pose cible pour le rig.
 * La base ne contient QUE les entrées : chaque comportement gère ses vues, ses angles et ses bornes.
 * Choisi dans le Details du rig (propriété Behavior).
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced, CollapseCategories)
class PLAGUETALEGO_API UBoardCameraBehavior : public UObject
{
	GENERATED_BODY()

public:
	// Appelés par le rig au BeginPlay / EndPlay
	void Activate(ABoardCameraRig* InRig);
	void Deactivate();

	// --- Entrées, communes à tous les comportements (la base ne fait rien) ---

	// Cran de molette : +1 vers le haut, -1 vers le bas
	virtual void HandleViewStep(int32 Step) {}

	// Clic glissé (le controller a déjà vérifié que le clic n'était pas sur une case jouable)
	virtual void HandleDragStart() {}
	virtual void HandleDrag(const FVector2D& MouseDelta) {}
	virtual void HandleDragEnd() {}

	// Le pion joueur vient de terminer un déplacement sur cette case
	virtual void HandlePlayerArrived(ABoardNode* Node) {}

	// Permet aux comportements d'utiliser GetWorld() (timers, subsystems…)
	virtual UWorld* GetWorld() const override;

protected:
	// À surcharger : appelés après que le rig est connu, et avant qu'il soit oublié
	virtual void OnActivated() {}
	virtual void OnDeactivated() {}

	ABoardCameraRig* GetRig() const { return Rig; }

	// Raccourci : Rig->SetTargetPose
	void SetTargetPose(const FBoardCameraPose& Pose, bool bInstant = false);

	// Raccourci : pose de cadrage du rig (pivot et distance), sur laquelle chaque comportement règle ses angles
	FBoardCameraPose GetFramingPose() const;

private:
	UPROPERTY(Transient)
	TObjectPtr<ABoardCameraRig> Rig;
};
