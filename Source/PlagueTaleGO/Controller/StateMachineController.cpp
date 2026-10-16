// Fill out your copyright notice in the Description page of Project Settings.


#include "StateMachineController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"

#include "PlagueTaleGO/GameMode/StateMachineGameMode.h"

void AStateMachineController::BeginPlay()
{
	Super::BeginPlay();

	/* Add Mapping Context */
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AStateMachineController::SetupInputComponent()
{
	Super::SetupInputComponent();

	/* Bind Actions */
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (JumpInputAction)
		{
			EnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Started, this, &AStateMachineController::PressJump);
		}
	}
}

void AStateMachineController::PressJump()
{
	
	if (AStateMachineGameMode* SMgameMode = Cast<AStateMachineGameMode>(GetWorld()->GetAuthGameMode()))
	{
		SMgameMode->GoToNextState();
	}


}
