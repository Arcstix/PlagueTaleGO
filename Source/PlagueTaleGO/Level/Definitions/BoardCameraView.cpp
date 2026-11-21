// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardCameraView.h"

void UBoardCameraView::ApplyTo(FBoardCameraPose& Pose) const
{
	Pose.Pitch = Pitch;
	Pose.Yaw = Yaw;
	Pose.ArmLength = ArmLength;
}
