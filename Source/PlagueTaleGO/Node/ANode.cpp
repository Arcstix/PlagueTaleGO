// Fill out your copyright notice in the Description page of Project Settings.


#include "ANode.h"
#include "AConnection.h"

// Sets default values
AANode::AANode()
{
	// Les nœuds n'ont pas besoin de Tick
	PrimaryActorTick.bCanEverTick = false;
}

void AANode::AddAdjacent(AANode* Other)
{
	if (!Other || Other == this)
	{
		return;
	}
	m_adjacentNodes.AddUnique(Other);
}

bool AANode::IsOccupied() const
{
	return bIsOccupied;
}

void AANode::SetOccupied(bool bValue)
{
	bIsOccupied = bValue;
}

bool AANode::IsBlocked() const
{
	return bIsBlocked;
}

void AANode::SetBlocked(bool bValue)
{
	bIsBlocked = bValue;
}

bool AANode::IsSwipable() const
{
	return bIsSwipable;
}

void AANode::SetSwipable(bool bValue)
{
	bIsSwipable = bValue;
}

bool AANode::IsClickable() const
{
	return bIsClickable;
}

void AANode::SetClickable(bool bValue)
{
	bIsClickable = bValue;
}

// Called when the game starts or when spawned
void AANode::BeginPlay()
{
	Super::BeginPlay();
	
}

void AANode::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (m_connections.Num() == 0) return;

	for (AAConnection* Connection : m_connections)
	{
		if (Connection != nullptr)
			Connection->ConnectNode();
	}
}

// Called every frame
void AANode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AANode::IsConnectedTo(AANode* node)
{
	if (m_adjacentNodes.Contains(node))
	{
		return true;
	}
	
	return false;
}

