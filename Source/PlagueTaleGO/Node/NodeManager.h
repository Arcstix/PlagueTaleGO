// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NodeManager.generated.h"

class AANode;
/**
 * 
 */
UCLASS()
class PLAGUETALEGO_API UNodeManager : public UObject
{
	GENERATED_BODY()

private:
	TArray<AANode*> m_Nodes;

public:
	TArray<AANode*> GetNodes();
	
	
};
