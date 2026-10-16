// BoardPlayerController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BoardPlayerController.generated.h"

class APlayerPawn;
class ABoardPawn;

UCLASS()
class PLAGUETALEGO_API ABoardPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;

private:
	void HandleClick();

	// UFUNCTION obligatoire pour pouvoir utiliser AddDynamic
	UFUNCTION()
	void HandlePlayerMoveFinished(ABoardPawn* MovedPawn);

	UPROPERTY()
	TObjectPtr<APlayerPawn> PlayerPawn;
};
