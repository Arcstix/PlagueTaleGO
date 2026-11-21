// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BoardCameraBehavior.h"
#include "HitmanCameraBehavior.generated.h"

class UBoardCameraView;

/**
 * Caméra façon Hitman GO : deux modes, Default (penché) et Top (vue de dessus).
 * Molette vers le haut : Default → Top. Molette vers le bas : Top → Default.
 * La vue Top garde le lacet et la distance de la vue Default actuelle.
 */
UCLASS(meta = (DisplayName = "Hitman GO"))
class PLAGUETALEGO_API UHitmanCameraBehavior : public UBoardCameraBehavior
{
	GENERATED_BODY()

public:
	virtual void HandleViewStep(int32 Step) override;

	virtual void HandleDragStart() override;
	virtual void HandleDrag(const FVector2D& MouseDelta) override;
	virtual void HandleDragEnd() override;
	virtual void HandlePlayerArrived(ABoardNode* Node) override;

protected:
	virtual void OnActivated() override;

	// Vue par défaut au lancement (les cases pourront la remplacer, tâche 27)
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (
		ToolTip = "Default view at start. Empty = the rig's start pose."))
	TObjectPtr<UBoardCameraView> DefaultView;

	// Tangage de la vue Top (pas -90 : le lacet ne serait plus défini)
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (
		ClampMin = "-89.9", ClampMax = "0", Units = "Degrees",
		ToolTip = "Vertical angle of the top view. Keeps the yaw and distance of the current default view."))
	float TopPitch = -80.f;

	// --- Pivot au clic glissé ---

	UPROPERTY(EditAnywhere, Category = "Camera|Drag", meta = (
		ClampMin = "0", ClampMax = "180", Units = "Degrees",
		ToolTip = "Maximum horizontal angle away from the current view while dragging."))
	float MaxYawOffset = 20.f;

	UPROPERTY(EditAnywhere, Category = "Camera|Drag", meta = (
		ClampMin = "0", ClampMax = "89", Units = "Degrees",
		ToolTip = "Maximum vertical angle away from the current view while dragging."))
	float MaxPitchOffset = 10.f;

	UPROPERTY(EditAnywhere, Category = "Camera|Drag", meta = (
		ToolTip = "Degrees per unit of horizontal mouse movement. Negative = inverted."))
	float YawSensitivity = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Camera|Drag", meta = (
		ToolTip = "Degrees per unit of vertical mouse movement. Negative = inverted."))
	float PitchSensitivity = 0.3f;

private:
	enum class EMode : uint8
	{
		Default,
		Top
	};

	// Pose de la vue Default en cours (pivot du rig + CurrentDefaultView)
	FBoardCameraPose GetDefaultPose() const;
	
	// Pose de référence du mode actuel (sans décalage de glisser)
	FBoardCameraPose GetReferencePose() const;

	// Envoie la pose de référence au rig
	void UpdateTarget(bool bInstant = false);

	EMode Mode = EMode::Default;
	
	// Lacet et distance figés en entrant en vue Top : un changement de vue Default ne bouge pas la caméra en Top
	float TopYaw = 0.f;
	float TopArmLength = 0.f;

	// Décalage du glisser, ajouté à la pose de référence ; remis à zéro au relâchement
	float YawOffset = 0.f;
	float PitchOffset = 0.f;
	bool bDragging = false;

	// Vue Default en cours : DefaultView au départ, puis celle des cases (tâche 27)
	UPROPERTY(Transient)
	TObjectPtr<UBoardCameraView> CurrentDefaultView;
};
