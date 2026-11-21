// Fill out your copyright notice in the Description page of Project Settings.


#include "FixedCameraBehavior.h"
#include "PlagueTaleGO/Level/Definitions/BoardCameraView.h"

void UFixedCameraBehavior::OnActivated()
{
	FBoardCameraPose Pose = GetFramingPose();   // pivot = position du rig
	if (View)
	{
		View->ApplyTo(Pose);
	}
	SetTargetPose(Pose, true);   // pas de transition au lancement
}
