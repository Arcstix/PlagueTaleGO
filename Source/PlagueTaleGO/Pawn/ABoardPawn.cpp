// BoardPawn.cpp
#include "ABoardPawn.h"
#include "../Node/ANode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "PlagueTaleGO/Node/BoardSubsystem.h"

ABoardPawn::ABoardPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	// Tick désactivé au départ, activé seulement pendant un déplacement
	PrimaryActorTick.bStartWithTickEnabled = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	// L'entité ne doit pas intercepter le clic destiné au nœud en dessous
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABoardPawn::SnapToNode(AANode* Node)
{
	if (!Node)
	{
		return;
	}

	CurrentNode = Node;
	SetActorLocation(Node->GetActorLocation());

	if (UBoardSubsystem* Board = GetWorld()->GetSubsystem<UBoardSubsystem>())
	{
		Board->RegisterPawn(this, Node);
	}
}

void ABoardPawn::BeginMove(AANode* InTargetNode)
{
	if (!InTargetNode || IsMoving())
	{
		return;
	}

	TargetNode = InTargetNode;
	MoveStartLocation = GetActorLocation();
	MoveTargetLocation = TargetNode->GetActorLocation();
	MoveAlpha = 0.f;
	State = EBoardPawnState::Moving;

	SetActorTickEnabled(true);
}

void ABoardPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsMoving())
	{
		return;
	}

	MoveAlpha += DeltaSeconds / FMath::Max(MoveDuration, KINDA_SMALL_NUMBER);

	if (MoveAlpha >= 1.f)
	{
		SetActorLocation(MoveTargetLocation);
		CompleteMove();
	}
	else
	{
		SetActorLocation(FMath::Lerp(MoveStartLocation, MoveTargetLocation, MoveAlpha));
	}
}

void ABoardPawn::CompleteMove()
{
	LastNode = CurrentNode;
	CurrentNode = TargetNode;
	TargetNode = nullptr;
	State = EBoardPawnState::Idle;

	SetActorTickEnabled(false);
	OnMoveFinished.Broadcast(this);
}