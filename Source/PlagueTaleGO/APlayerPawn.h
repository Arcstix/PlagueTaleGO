// PlayerEntity.h
#pragma once

#include "CoreMinimal.h"
#include "ABoardPawn.h"
#include "BoardTypes.h"
#include "APlayerPawn.generated.h"

class ABoardNode;

UCLASS()
class APlayerPawn : public ABoardPawn
{
	GENERATED_BODY()

public:
	// Appelée par le contrôleur joueur quand une case est choisie.
	// Transmet la demande au Board, qui décide.
	// UFUNCTION(BlueprintCallable, Category = "Board|Player")
	// EMoveResult RequestMove(ABoardNode* InTargetNode);
};