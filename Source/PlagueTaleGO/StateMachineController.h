// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StateMachineController.generated.h"
class UInputAction;
class UInputMappingContext;


/*
 * 
 */
UCLASS()
class PLAGUETALEGO_API AStateMachineController : public APlayerController
{
	GENERATED_BODY()
	
	protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpInputAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "MappingContext")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	virtual void BeginPlay() override;
	
	virtual  void SetupInputComponent() override;
	
	public:
	void PressJump();
};
