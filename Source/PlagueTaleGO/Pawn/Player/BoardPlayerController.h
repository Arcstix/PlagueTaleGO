// BoardPlayerController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BoardPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class APlayerPawn;
class ABoardPawn;

UCLASS()
class PLAGUETALEGO_API ABoardPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpInputAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "MappingContext")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	virtual void SetupInputComponent() override;
	
private:
	void HandleClick();
	void TryBindPlayerPawn();

	// UFUNCTION obligatoire pour pouvoir utiliser AddDynamic
	UFUNCTION()
	void HandlePlayerMoveFinished(ABoardPawn* MovedPawn);

	UPROPERTY()
	TObjectPtr<APlayerPawn> PlayerPawn;

public:
	void PressJump();
};
