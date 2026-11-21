// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardCameraBehavior.h"
#include "PlagueTaleGO/Camera/BoardCameraRig.h"

void UBoardCameraBehavior::Activate(ABoardCameraRig* InRig)
{
	Rig = InRig;
	if (Rig)
	{
		OnActivated();
	}
}

void UBoardCameraBehavior::Deactivate()
{
	if (Rig)
	{
		OnDeactivated();
	}
	Rig = nullptr;
}

UWorld* UBoardCameraBehavior::GetWorld() const
{
	return Rig ? Rig->GetWorld() : nullptr;
}

void UBoardCameraBehavior::SetTargetPose(const FBoardCameraPose& Pose, bool bInstant)
{
	if (Rig)
	{
		Rig->SetTargetPose(Pose, bInstant);
	}
}

FBoardCameraPose UBoardCameraBehavior::GetFramingPose() const
{
	return Rig ? Rig->GetFramingPose() : FBoardCameraPose();
}
