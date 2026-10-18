// BoardPlayerController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BoardPlayerController.generated.h"

class APlayerPawn;
class ABoardPawn;
class UInputAction;
class UInputMappingContext;

UCLASS()
class PLAGUETALEGO_API ABoardPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> PrimaryInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SecondaryInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "MappingContext")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

public:
	// Font avancer la state machine (touches Primary / Secondary)
	void PrimaryAction();
	void SecondaryAction();

private:
	void HandleClick();
	void TryBindPlayerPawn();

	// UFUNCTION obligatoire pour pouvoir utiliser AddDynamic
	UFUNCTION()
	void HandlePlayerMoveFinished(ABoardPawn* MovedPawn);

	UPROPERTY()
	TObjectPtr<APlayerPawn> PlayerPawn;
};
