// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerSpawnNode.h"


// Sets default values
APlayerSpawnNode::APlayerSpawnNode()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APlayerSpawnNode::BeginPlay()
{
	Super::BeginPlay();

	if (BP_PlayerToSpawn != nullptr)
	{
		APlayerPawn* SpawnedPawn = GetWorld()->SpawnActor<APlayerPawn>(
			BP_PlayerToSpawn,
			GetActorLocation(),
			GetActorRotation());

		if (SpawnedPawn)
		{
			SpawnedPawn->SnapToNode(this);
		}
	}
}

// Called every frame
void APlayerSpawnNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
