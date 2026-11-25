// BoardPawn.cpp
#include "ABoardPawn.h"
#include "PlagueTaleGO/Board/BoardNode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "PlagueTaleGO/Board/BoardSubsystem.h"
#include "PlagueTaleGO/Audio/BoardAudioSubsystem.h"
// Include complet : convertir UBoardNodeType* en UBoardDefinition* (contexte audio) exige la classe complète
#include "PlagueTaleGO/Level/Definitions/BoardNodeType.h"

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

void ABoardPawn::SnapToNode(ABoardNode* Node)
{
	if (!Node)
	{
		return;
	}
	
	SetActorLocation(Node->GetActorLocation());

	if (UBoardSubsystem* Board = GetWorld()->GetSubsystem<UBoardSubsystem>())
	{
		Board->RegisterPawn(this, Node);
	}
}

void ABoardPawn::BeginMove(ABoardNode* InTargetNode)
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
	PlayMoveSound(TargetNode);
}

ABoardNode* ABoardPawn::GetCurrentNode() const
{
	const UWorld* World = GetWorld();
	const UBoardSubsystem* Board = World ? World->GetSubsystem<UBoardSubsystem>() : nullptr;
	return Board ? Board->GetPawnNode(this) : nullptr;
}


void ABoardPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Le pion quitte le jeu : il ne doit plus occuper sa case
	if (UWorld* World = GetWorld())
	{
		if (UBoardSubsystem* Board = World->GetSubsystem<UBoardSubsystem>())
		{
			Board->UnregisterPawn(this);
		}
	}
	
	Super::EndPlay(EndPlayReason);
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

EMoveResult ABoardPawn::RequestMove(ABoardNode* InTargetNode)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return EMoveResult::Blocked;
	}

	UBoardSubsystem* Board = World->GetSubsystem<UBoardSubsystem>();
	if (!Board)
	{
		return EMoveResult::Blocked;
	}

	return Board->TryMovePawn(this, InTargetNode);
}

void ABoardPawn::CompleteMove()
{
	TargetNode = nullptr;
	State = EBoardPawnState::Idle;

	SetActorTickEnabled(false);
	OnMoveFinished.Broadcast(this);
}

void ABoardPawn::PlayMoveSound(const ABoardNode* DestinationNode) const
{
	if (!MoveSoundEvent.IsValid())
	{
		return;
	}
	if (UBoardAudioSubsystem* Audio = UBoardAudioSubsystem::Get(this))
	{
		// Le type de la case de destination sert de contexte : DA_Node_Wood peut remplacer le son par un pas sur du bois.
		// Son joué à la position de départ : en 3D, il ne suit pas le pion pendant le déplacement (court, sans effet audible).
		Audio->PlaySoundAtLocation(MoveSoundEvent, GetActorLocation(), DestinationNode ? DestinationNode->GetNodeType() : nullptr);
	}
}