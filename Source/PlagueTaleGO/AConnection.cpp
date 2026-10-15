// Fill out your copyright notice in the Description page of Project Settings.


#include "AConnection.h"
#include "ANode.h"

// Sets default values
AAConnection::AAConnection()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NodeMesh"));
	StaticMesh->SetupAttachment(RootComponent);
	
}

// Called when the game starts or when spawned
void AAConnection::BeginPlay()
{
	Super::BeginPlay();
}

void AAConnection::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (node1 == nullptr || node2 == nullptr) return;

	ConnectNode();
}

// Called every frame
void AAConnection::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

AANode* AAConnection::GetNode1() const
{
	return node1;
}

AANode* AAConnection::GetNode2() const
{
	return node2;
}

void AAConnection::ConnectNode()
{
	if (node1 == nullptr || node2 == nullptr) return;
	
	FVector Start = GetNode1()->GetActorLocation() + GetNode1()->GetActorScale().X * m_offsetFromNode;
	FVector End = GetNode2()->GetActorLocation() + GetNode2()->GetActorScale().X * m_offsetFromNode;

	const FVector Direction = End - Start;
	const float Distance = Direction.Length();
	const FVector Midpoint = (Start + End) * 0.5f;

	if (!StaticMesh)
	{
		return;
	}

	if (Distance <= KINDA_SMALL_NUMBER)
	{
		StaticMesh->SetVisibility(false);
		return;
	}

	StaticMesh->SetVisibility(true);
	
	StaticMesh->SetWorldLocation(Midpoint);
	StaticMesh->SetWorldRotation(Direction.Rotation());
	StaticMesh->SetWorldScale3D(
		FVector(Distance / 100.0f, 0.1f, 0.1f));
}

