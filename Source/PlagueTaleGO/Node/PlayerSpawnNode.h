// PlayerSpawnNode.h
#pragma once

#include "CoreMinimal.h"
#include "ANode.h"
#include "../Pawn/Player/APlayerPawn.h" // adapter le chemin
#include "PlayerSpawnNode.generated.h"

UCLASS()
class PLAGUETALEGO_API APlayerSpawnNode : public AANode
{
	GENERATED_BODY()

public:
	APlayerSpawnNode();

	// Blueprint dérivé de APlayerPawn, créé sur cette case au démarrage
	UPROPERTY(EditAnywhere, Category = "0 - Spawn")
	TSubclassOf<APlayerPawn> BP_PlayerToSpawn;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};