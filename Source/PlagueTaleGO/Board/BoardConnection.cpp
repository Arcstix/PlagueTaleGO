// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardConnection.h"
#include "BoardNode.h"

// Sets default values
ABoardConnection::ABoardConnection()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NodeMesh"));
	StaticMesh->SetupAttachment(RootComponent);
	
}

// Called when the game starts or when spawned
void ABoardConnection::BeginPlay()
{
	Super::BeginPlay();
}

void ABoardConnection::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (NodeA == nullptr || NodeB == nullptr) return;

	ConnectNode();
}

// Called every frame
void ABoardConnection::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

ABoardNode* ABoardConnection::GetNode1() const
{
	return NodeA;
}

ABoardNode* ABoardConnection::GetNode2() const
{
	return NodeB;
}

void ABoardConnection::ConnectNode()
{
	if (!NodeA || !NodeB || !StaticMesh) return;
	
	NodeA->AddAdjacent(NodeB);
	NodeB->AddAdjacent(NodeA);
	
	FVector Start = GetNode1()->GetActorLocation();
	FVector End = GetNode2()->GetActorLocation();

	const FVector Direction = End - Start;
	const float Distance = Direction.Length() - OffsetFromNode;
	FVector Midpoint = (Start + End) * 0.5f;
	Midpoint.Z = ZOffset;

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

