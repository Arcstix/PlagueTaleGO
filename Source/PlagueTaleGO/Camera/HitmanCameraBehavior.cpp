// Fill out your copyright notice in the Description page of Project Settings.


#include "HitmanCameraBehavior.h"

#include "PlagueTaleGO/Board/BoardNode.h"
#include "PlagueTaleGO/Level/Definitions/BoardCameraView.h"

void UHitmanCameraBehavior::OnActivated()
{
	Mode = EMode::Default;
	CurrentDefaultView = DefaultView;
	YawOffset = 0.f;
	PitchOffset = 0.f;
	bDragging = false;
	UpdateTarget(true);   // pas de transition au lancement
}

void UHitmanCameraBehavior::HandleViewStep(int32 Step)
{
	EMode NewMode = Mode;
	if (Step > 0 && Mode == EMode::Default)
	{
		NewMode = EMode::Top;
	}
	else if (Step < 0 && Mode == EMode::Top)
	{
		NewMode = EMode::Default;
	}

	if (NewMode == Mode)
	{
		return;   // déjà en Top et molette vers le haut, ou déjà en Default et molette vers le bas
	}

	if (NewMode == EMode::Top)
	{
		// La vue Top reprend le lacet et la distance de la vue Default au moment du passage
		const FBoardCameraPose DefaultPose = GetDefaultPose();
		TopYaw = DefaultPose.Yaw;
		TopArmLength = DefaultPose.ArmLength;
	}
	
	Mode = NewMode;
	UpdateTarget();
}

void UHitmanCameraBehavior::HandleDragStart()
{
	bDragging = true;
}

void UHitmanCameraBehavior::HandleDrag(const FVector2D& MouseDelta)
{
	if (!bDragging)
	{
		return;
	}

	YawOffset = FMath::Clamp(YawOffset + MouseDelta.X * YawSensitivity, -MaxYawOffset, MaxYawOffset);

	// En vue Top, on ne peut pas descendre sous la verticale : la borne basse est rapprochée
	const float ReferencePitch = GetReferencePose().Pitch;
	const float MinPitchOffset = FMath::Max(-MaxPitchOffset, -89.9f - ReferencePitch);
	PitchOffset = FMath::Clamp(PitchOffset + MouseDelta.Y * PitchSensitivity, MinPitchOffset, MaxPitchOffset);

	UpdateTarget();
}

void UHitmanCameraBehavior::HandleDragEnd()
{
	bDragging = false;
	YawOffset = 0.f;
	PitchOffset = 0.f;
	UpdateTarget();   // retour en douceur à la rotation de référence de la vue actuelle
}

void UHitmanCameraBehavior::HandlePlayerArrived(ABoardNode* Node)
{
	UBoardCameraView* CellView = Node ? Node->GetCameraView() : nullptr;
	if (!CellView || CellView == CurrentDefaultView)
	{
		return;   // case sans point de vue, ou même vue : la caméra garde la sienne
	}

	CurrentDefaultView = CellView;

	// En Default : transition en douceur. En Top : rien ne bouge, la vue servira au retour en Default.
	if (Mode == EMode::Default)
	{
		UpdateTarget();
	}
}

FBoardCameraPose UHitmanCameraBehavior::GetDefaultPose() const
{
	FBoardCameraPose Pose = GetFramingPose();   // pivot = position du rig
	if (CurrentDefaultView)
	{
		CurrentDefaultView->ApplyTo(Pose);
	}
	return Pose;
}

FBoardCameraPose UHitmanCameraBehavior::GetReferencePose() const
{
	FBoardCameraPose Pose = GetDefaultPose();
	if (Mode == EMode::Top)
	{
		Pose.Pitch = TopPitch;
		Pose.Yaw = TopYaw;             // valeurs figées à l'entrée en Top
		Pose.ArmLength = TopArmLength;
	}
	return Pose;
}

void UHitmanCameraBehavior::UpdateTarget(bool bInstant)
{
	FBoardCameraPose Pose = GetReferencePose();
	Pose.Yaw += YawOffset;
	Pose.Pitch = FMath::Clamp(Pose.Pitch + PitchOffset, -89.9f, 89.9f);   // jamais au-delà de la verticale
	SetTargetPose(Pose, bInstant);
}
