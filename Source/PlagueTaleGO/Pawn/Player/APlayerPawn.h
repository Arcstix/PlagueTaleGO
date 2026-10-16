// PlayerPawn.h
#pragma once

#include "CoreMinimal.h"
#include "../ABoardPawn.h"
#include "../../Node/BoardTypes.h"
#include "APlayerPawn.generated.h"

class AANode;

UCLASS()
class PLAGUETALEGO_API APlayerPawn : public ABoardPawn
{
	GENERATED_BODY()

public:
	// Demande au Board la permission de se déplacer
	UFUNCTION(BlueprintCallable, Category = "Board|Player")
	EMoveResult RequestMove(AANode* InTargetNode);
};