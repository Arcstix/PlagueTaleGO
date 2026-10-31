// PlayerSpawnNode.h
#pragma once

#include "CoreMinimal.h"
#include "BoardNode.h"
#include "PlagueTaleGO/Entities/ABoardPawn.h"
#include "EntitySpawnNode.generated.h"

UCLASS()
class PLAGUETALEGO_API AEntitySpawnNode : public ABoardNode
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEntitySpawnNode();

	// Blueprint dérivé de ABoardPawn, créé sur cette case au démarrage
	UPROPERTY(EditAnywhere, Category = "0 - Spawn")
	TSubclassOf<ABoardPawn> BP_EntityToSpawn;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};