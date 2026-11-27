#include "BlueDustTrailComponent.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "PlagueTaleGO/PlagueTaleGO.h"
#include "PlagueTaleGO/Board/BoardNode.h"
#include "PlagueTaleGO/Board/BoardSubsystem.h"
#include "PlagueTaleGO/Entities/ABoardPawn.h"

UBlueDustTrailComponent::UBlueDustTrailComponent()
{
	// Le Tick ne sert qu'au dessin de debug : coupé au départ, activé dans BeginPlay si bDrawDebug (hors Shipping)
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// ---------------------------------------------------------------------------
// Cycle de vie
// ---------------------------------------------------------------------------
void UBlueDustTrailComponent::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	UBoardSubsystem* Board = World ? World->GetSubsystem<UBoardSubsystem>() : nullptr;
	if (!Board)
	{
		UE_LOG(LogPlagueTaleGO, Warning, TEXT("[BlueDust] Pas de UBoardSubsystem : la trace de %s restera vide."),
			*GetNameSafe(GetOwner()));
		return;
	}

	// Le propriétaire est créé par le GameMode après la construction du graphe : le plateau existe déjà ici
	BoardSubsystem = Board;
	Board->OnPawnMoved.AddDynamic(this, &UBlueDustTrailComponent::HandlePawnMoved);

#if !UE_BUILD_SHIPPING
	SetComponentTickEnabled(bDrawDebug);
#endif
}

void UBlueDustTrailComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Toujours se désabonner : le plateau peut survivre au propriétaire
	if (UBoardSubsystem* Board = BoardSubsystem.Get())
	{
		Board->OnPawnMoved.RemoveDynamic(this, &UBlueDustTrailComponent::HandlePawnMoved);
	}
	BoardSubsystem.Reset();
	Traces.Reset();

	Super::EndPlay(EndPlayReason);
}

void UBlueDustTrailComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if !UE_BUILD_SHIPPING
	UWorld* World = GetWorld();
	if (!World || !bDrawDebug)
	{
		return;
	}

	// Durée 0 + redessiné à chaque frame : le dessin suit toujours l'état réel des traces
	for (const FBlueDustTrace& Trace : Traces)
	{
		if (!Trace.Node)
		{
			continue;
		}
		const FVector Location = Trace.Node->GetActorLocation() + FVector(0.f, 0.f, DebugRadius);
		DrawDebugSphere(World, Location, DebugRadius, 12, DebugColor, false, 0.f, 0, 2.f);
		DrawDebugString(World, Location + FVector(0.f, 0.f, DebugRadius * 1.5f),
			FString::FromInt(Trace.RemainingLifetime), nullptr, FColor::White, 0.f, true);
	}
#endif
}

// ---------------------------------------------------------------------------
// Règles
// ---------------------------------------------------------------------------
void UBlueDustTrailComponent::HandlePawnMoved(ABoardPawn* Pawn, ABoardNode* FromNode, ABoardNode* ToNode)
{
	// Le plateau diffuse les déplacements de TOUS les pions : seul le propriétaire laisse cette trace
	if (!Pawn || Pawn != GetOwner())
	{
		return;
	}

	// L'ordre compte : vieillir d'abord, sinon la nouvelle trace perdrait 1 dès sa pose
	AgeTraces();
	RemoveTraceInternal(ToNode, EBlueDustRemovalReason::OwnerReturned);
	LeaveTrace(FromNode);

	OnTrailUpdated.Broadcast(this);
}

void UBlueDustTrailComponent::AgeTraces()
{
	// Parcours à l'envers : on peut retirer des éléments sans décaler ceux qui restent à traiter
	for (int32 Index = Traces.Num() - 1; Index >= 0; --Index)
	{
		--Traces[Index].RemainingLifetime;
		if (Traces[Index].RemainingLifetime <= 0)
		{
			const FBlueDustTrace Expired = Traces[Index];   // copie : l'élément disparaît du tableau juste après
			Traces.RemoveAt(Index);
			UE_LOG(LogPlagueTaleGO, Verbose, TEXT("[BlueDust] Trace expirée sur %s"), *GetNameSafe(Expired.Node));
			OnTraceRemoved.Broadcast(Expired, EBlueDustRemovalReason::Expired);
		}
	}
}

void UBlueDustTrailComponent::LeaveTrace(ABoardNode* Node)
{
	if (!Node || TrailLength <= 0)
	{
		return;
	}

	// Une seule trace par case. Dans le jeu normal ce cas n'arrive pas (la trace sous le propriétaire
	// est retirée quand il arrive) : on rafraîchit par sécurité, sans dupliquer ni rediffuser l'apparition.
	const int32 Existing = FindTraceIndex(Node);
	if (Existing != INDEX_NONE)
	{
		FBlueDustTrace Refreshed = Traces[Existing];
		Refreshed.RemainingLifetime = TrailLength;
		Traces.RemoveAt(Existing);
		Traces.Insert(Refreshed, 0);
		UE_LOG(LogPlagueTaleGO, Verbose, TEXT("[BlueDust] Trace rafraîchie sur %s"), *GetNameSafe(Node));
		return;
	}

	FBlueDustTrace NewTrace;
	NewTrace.Node = Node;
	NewTrace.RemainingLifetime = TrailLength;
	Traces.Insert(NewTrace, 0);   // index 0 = la plus récente (N-1)

	UE_LOG(LogPlagueTaleGO, Verbose, TEXT("[BlueDust] Trace posée sur %s (durée %d)"), *GetNameSafe(Node), TrailLength);
	OnTraceAdded.Broadcast(NewTrace);
}

bool UBlueDustTrailComponent::RemoveTraceInternal(const ABoardNode* Node, EBlueDustRemovalReason Reason)
{
	const int32 Index = FindTraceIndex(Node);
	if (Index == INDEX_NONE)
	{
		return false;
	}

	const FBlueDustTrace Removed = Traces[Index];
	Traces.RemoveAt(Index);
	UE_LOG(LogPlagueTaleGO, Verbose, TEXT("[BlueDust] Trace retirée sur %s (%s)"),
		*GetNameSafe(Removed.Node), *UEnum::GetValueAsString(Reason));
	OnTraceRemoved.Broadcast(Removed, Reason);
	return true;
}

int32 UBlueDustTrailComponent::FindTraceIndex(const ABoardNode* Node) const
{
	if (!Node)
	{
		return INDEX_NONE;
	}
	return Traces.IndexOfByPredicate([Node](const FBlueDustTrace& Trace) { return Trace.Node.Get() == Node; });
}
