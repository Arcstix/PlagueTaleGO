// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ANode.h"
#include "EntitySpawnNode.generated.h"

UCLASS()
class PLAGUETALEGO_API AEntitySpawnNode : public AANode
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEntitySpawnNode();

	UPROPERTY(EditAnywhere, Category = "0 - Spawn")
	TSubclassOf<APawn> BP_EntityToSpawn;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
