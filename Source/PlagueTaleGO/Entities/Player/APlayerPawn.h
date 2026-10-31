// PlayerPawn.h
#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Entities/ABoardPawn.h"
#include "PlagueTaleGO/Board/BoardTypes.h"
#include "APlayerPawn.generated.h"

class ABoardNode;

UCLASS()
class PLAGUETALEGO_API APlayerPawn : public ABoardPawn
{
	GENERATED_BODY()

public:
	// Demande au Board la permission de se déplacer
	UFUNCTION(BlueprintCallable, Category = "Board|Player")
	EMoveResult RequestMove(ABoardNode* InTargetNode);
};