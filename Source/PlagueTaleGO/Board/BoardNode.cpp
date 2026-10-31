// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardNode.h"
#include "BoardConnection.h"

// Sets default values
ABoardNode::ABoardNode()
{
	// Les nœuds n'ont pas besoin de Tick
	PrimaryActorTick.bCanEverTick = false;
}

void ABoardNode::AddAdjacent(ABoardNode* Other)
{
	if (!Other || Other == this)
	{
		return;
	}
	AdjacentNodes.AddUnique(Other);
}

bool ABoardNode::IsOccupied() const
{
	return bIsOccupied;
}

void ABoardNode::SetOccupied(bool bValue)
{
	bIsOccupied = bValue;
}

bool ABoardNode::IsBlocked() const
{
	return bIsBlocked;
}

void ABoardNode::SetBlocked(bool bValue)
{
	bIsBlocked = bValue;
}

bool ABoardNode::IsSwipable() const
{
	return bIsSwipable;
}

void ABoardNode::SetSwipable(bool bValue)
{
	bIsSwipable = bValue;
}

bool ABoardNode::IsClickable() const
{
	return bIsClickable;
}

void ABoardNode::SetClickable(bool bValue)
{
	bIsClickable = bValue;
}

// Called when the game starts or when spawned
void ABoardNode::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABoardNode::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (Connections.Num() == 0) return;

	for (ABoardConnection* Connection : Connections)
	{
		if (Connection != nullptr)
			Connection->ConnectNode();
	}
}

// Called every frame
void ABoardNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ABoardNode::IsConnectedTo(ABoardNode* node)
{
	if (AdjacentNodes.Contains(node))
	{
		return true;
	}
	
	return false;
}

