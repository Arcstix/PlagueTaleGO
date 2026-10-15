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
			EnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Started, this, &AStateMachineController::PrimaryAction);
		}
		if (OtherInputAction)
		{
			EnhancedInputComponent->BindAction(OtherInputAction, ETriggerEvent::Started, this, &AStateMachineController::SecondaryAction);
		}
	}
}

void AStateMachineController::PrimaryAction()
{
	
	if (AStateMachineGameMode* SMgameMode = Cast<AStateMachineGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (UStateBase* actualState=SMgameMode->GetActualState())
		{
			actualState->PrimaryInput();
		
			SMgameMode->GoToNextState();
		}
		
		
	}
	
}

void AStateMachineController::SecondaryAction()
{
	
	if (AStateMachineGameMode* SMgameMode = Cast<AStateMachineGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (UStateBase* actualState=SMgameMode->GetActualState())
		{
			actualState->SecondaryInput();
		
			SMgameMode->GoToNextState();
		}
		
		
		
	}
	
}
