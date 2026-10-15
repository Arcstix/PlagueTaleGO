// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NodeGameMode.generated.h"

class AANode;

/**
 * 
 */
UCLASS()
class PLAGUETALEGO_API ANodeGameMode : public AGameModeBase
{
	GENERATED_BODY()

private:
	TArray<AANode*> m_Nodes;
	
	
};
