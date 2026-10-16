// PlayerSpawnNode.h
#pragma once

#include "CoreMinimal.h"
#include "ANode.h"
#include "PlayerSpawnNode.generated.h"

UCLASS()
class PLAGUETALEGO_API AEntitySpawnNode : public AANode
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APlayerSpawnNode();

	// Blueprint dérivé de APlayerPawn, créé sur cette case au démarrage
	UPROPERTY(EditAnywhere, Category = "0 - Spawn")
	TSubclassOf<AActor> BP_PlayerToSpawn;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};