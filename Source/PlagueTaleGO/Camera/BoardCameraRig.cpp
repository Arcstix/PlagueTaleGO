// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardCameraRig.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"

namespace
{
	// En dessous de ces écarts, la cible est considérée comme atteinte
	constexpr float PivotTolerance = 0.5f;       // cm
	constexpr float AngleTolerance = 0.05f;      // degrés
	constexpr float ArmLengthTolerance = 0.5f;   // cm
}

// Sets default values
ABoardCameraRig::ABoardCameraRig()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;   // tick seulement pendant un mouvement

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Root);
	SpringArm->bDoCollisionTest = false;          // le décor ne rapproche jamais la caméra
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bEnableCameraLag = false;          // le lissage est fait par le rig
	SpringArm->bEnableCameraRotationLag = false;
	SpringArm->SetUsingAbsoluteRotation(true);    // l'orientation ne dépend que de la pose

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	// Trouvé par ABoardPlayerController::SetupBoardCamera
	Tags.Add(TEXT("BoardCamera"));
}

void ABoardCameraRig::DebugToggleTopView()
{
	if (!HasActorBegunPlay())
	{
		return;   // uniquement en PIE
	}

	FBoardCameraPose Pose = TargetPose;
	Pose.Pitch = (Pose.Pitch < -80.f) ? StartPitch : -89.f;
	SetTargetPose(Pose);
}

// Called when the game starts or when spawned
void ABoardCameraRig::BeginPlay()
{
	Super::BeginPlay();
	// Le bras se met à jour après le rig dans la même image : pas d'image de retard
	SpringArm->PrimaryComponentTick.AddPrerequisite(this, PrimaryActorTick);

	SetTargetPose(MakeStartPose(), true);
}

void ABoardCameraRig::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Aperçu dans l'éditeur : la caméra est déjà à sa pose de départ
	ApplyArm(MakeStartPose());
}

FBoardCameraPose ABoardCameraRig::MakeStartPose() const
{
	FBoardCameraPose Pose;
	Pose.Pivot = GetActorLocation();
	Pose.Pitch = StartPitch;
	Pose.Yaw = StartYaw;
	Pose.ArmLength = StartArmLength;
	return Pose;
}

void ABoardCameraRig::ApplyPose(const FBoardCameraPose& Pose)
{
	SetActorLocation(Pose.Pivot);
	ApplyArm(Pose);
}

void ABoardCameraRig::ApplyArm(const FBoardCameraPose& Pose)
{
	SpringArm->SetWorldRotation(Pose.GetRotation());
	SpringArm->TargetArmLength = Pose.ArmLength;
}

bool ABoardCameraRig::HasReachedTarget() const
{
	return FVector::Dist(CurrentPose.Pivot, TargetPose.Pivot) <= PivotTolerance
		&& FMath::Abs(FRotator::NormalizeAxis(CurrentPose.Pitch - TargetPose.Pitch)) <= AngleTolerance
		&& FMath::Abs(FRotator::NormalizeAxis(CurrentPose.Yaw - TargetPose.Yaw)) <= AngleTolerance
		&& FMath::Abs(CurrentPose.ArmLength - TargetPose.ArmLength) <= ArmLengthTolerance;
}

// Called every frame
void ABoardCameraRig::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentPose.Pivot = FMath::VInterpTo(CurrentPose.Pivot, TargetPose.Pivot, DeltaTime, PivotSpeed);

	// RInterpTo prend le chemin le plus court : de 350° à 10°, la caméra passe par 0°
	const FRotator NewRotation = FMath::RInterpTo(CurrentPose.GetRotation(), TargetPose.GetRotation(), DeltaTime, RotationSpeed);
	CurrentPose.Pitch = NewRotation.Pitch;
	CurrentPose.Yaw = NewRotation.Yaw;

	CurrentPose.ArmLength = FMath::FInterpTo(CurrentPose.ArmLength, TargetPose.ArmLength, DeltaTime, ArmLengthSpeed);

	if (HasReachedTarget())
	{
		CurrentPose = TargetPose;     // on termine pile sur la cible
		SetActorTickEnabled(false);
	}

	ApplyPose(CurrentPose);
}

void ABoardCameraRig::SetTargetPose(const FBoardCameraPose& NewTarget, bool bInstant)
{
	TargetPose = NewTarget;

	if (bInstant)
	{
		CurrentPose = TargetPose;
		ApplyPose(CurrentPose);
		SetActorTickEnabled(false);
		return;
	}

	SetActorTickEnabled(!HasReachedTarget());
}

