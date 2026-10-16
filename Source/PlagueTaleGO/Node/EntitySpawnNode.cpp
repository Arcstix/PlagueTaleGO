// Fill out your copyright notice in the Description page of Project Settings.


#include "EntitySpawnNode.h"


// Sets default values
AEntitySpawnNode::AEntitySpawnNode()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AEntitySpawnNode::BeginPlay()
{
	Super::BeginPlay();

	if (BP_EntityToSpawn != nullptr)
	{
		GetWorld()->SpawnActor<APawn>(
			BP_EntityToSpawn,
			GetActorLocation(),
			GetActorRotation());
	}
}

// Called every frame
void AEntitySpawnNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

