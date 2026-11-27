#include "BlueDustTrailComponent.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
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

	// Se désabonner de la destruction des acteurs bloqueurs encore vivants
	for (const TWeakObjectPtr<UObject>& Blocker : EmissionBlockers)
	{
		if (AActor* BlockerActor = Cast<AActor>(Blocker.Get()))
		{
			BlockerActor->OnDestroyed.RemoveDynamic(this, &UBlueDustTrailComponent::HandleBlockerDestroyed);
		}
	}
	EmissionBlockers.Reset();

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

	// Filet de sécurité : oublie les bloqueurs détruits sans passer par OnDestroyed (ex. un UObject non acteur)
	RefreshBlockedState();

	// L'ordre compte : vieillir d'abord, sinon la nouvelle trace perdrait 1 dès sa pose
	AgeTraces();
	RemoveTraceInternal(ToNode, EBlueDustRemovalReason::OwnerReturned);
	if (!IsEmissionBlocked())
	{
		LeaveTrace(FromNode);
	}

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

bool UBlueDustTrailComponent::ClearTrailInternal(EBlueDustRemovalReason Reason)
{
	if (Traces.IsEmpty())
	{
		return false;
	}

	// On vide le tableau AVANT de diffuser : un abonné qui relit la trace la voit déjà vide
	const TArray<FBlueDustTrace> Removed = MoveTemp(Traces);
	Traces.Reset();
	for (const FBlueDustTrace& Trace : Removed)
	{
		OnTraceRemoved.Broadcast(Trace, Reason);
	}
	UE_LOG(LogPlagueTaleGO, Log, TEXT("[BlueDust] %d trace(s) retirée(s) (%s)"), Removed.Num(), *UEnum::GetValueAsString(Reason));
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

// ---------------------------------------------------------------------------
// Requêtes
// ---------------------------------------------------------------------------
int32 UBlueDustTrailComponent::GetTraceLifetime(const ABoardNode* Node) const
{
	const int32 Index = FindTraceIndex(Node);
	return Index != INDEX_NONE ? Traces[Index].RemainingLifetime : 0;
}

TArray<FBlueDustTrace> UBlueDustTrailComponent::FindTracesIn(const TArray<ABoardNode*>& Nodes) const
{
	TArray<FBlueDustTrace> Found;
	for (const FBlueDustTrace& Trace : Traces)
	{
		if (Nodes.Contains(Trace.Node.Get()))
		{
			Found.Add(Trace);
		}
	}
	return Found;
}

ABoardNode* UBlueDustTrailComponent::GetNextTowardOwner(const ABoardNode* Node) const
{
	const int32 Index = FindTraceIndex(Node);
	if (Index == INDEX_NONE)
	{
		return nullptr;
	}
	if (Index > 0)
	{
		return Traces[Index - 1].Node;   // un cran plus récent
	}

	// La plus récente (N-1) mène au propriétaire lui-même
	const ABoardPawn* OwnerPawn = Cast<ABoardPawn>(GetOwner());
	return OwnerPawn ? OwnerPawn->GetCurrentNode() : nullptr;
}

// ---------------------------------------------------------------------------
// Modifications
// ---------------------------------------------------------------------------
bool UBlueDustTrailComponent::RemoveTraceAt(ABoardNode* Node)
{
	if (!RemoveTraceInternal(Node, EBlueDustRemovalReason::Removed))
	{
		return false;
	}
	OnTrailUpdated.Broadcast(this);
	return true;
}

void UBlueDustTrailComponent::ClearTrail()
{
	if (ClearTrailInternal(EBlueDustRemovalReason::Cleared))
	{
		OnTrailUpdated.Broadcast(this);
	}
}

void UBlueDustTrailComponent::SetTrailLength(int32 NewLength)
{
	TrailLength = FMath::Max(NewLength, 0);

	if (TrailLength == 0)
	{
		ClearTrail();
		return;
	}

	// Raccourcir seulement : une trace ne doit jamais vivre plus longtemps que la nouvelle durée
	bool bChanged = false;
	for (FBlueDustTrace& Trace : Traces)
	{
		if (Trace.RemainingLifetime > TrailLength)
		{
			Trace.RemainingLifetime = TrailLength;
			bChanged = true;
		}
	}
	if (bChanged)
	{
		OnTrailUpdated.Broadcast(this);
	}
}

// ---------------------------------------------------------------------------
// Blocage
// ---------------------------------------------------------------------------
void UBlueDustTrailComponent::SuppressTrail(UObject* Source)
{
	if (!Source)
	{
		UE_LOG(LogPlagueTaleGO, Warning, TEXT("[BlueDust] SuppressTrail sans source : refusé (le blocage ne pourrait jamais être levé)."));
		return;
	}

	const bool bCleared = ClearTrailInternal(EBlueDustRemovalReason::Suppressed);
	AddEmissionBlocker(Source);
	if (bCleared)
	{
		OnTrailUpdated.Broadcast(this);
	}
}

void UBlueDustTrailComponent::AddEmissionBlocker(UObject* Source)
{
	if (!Source)
	{
		UE_LOG(LogPlagueTaleGO, Warning, TEXT("[BlueDust] AddEmissionBlocker sans source : ignoré."));
		return;
	}
	if (EmissionBlockers.ContainsByPredicate([Source](const TWeakObjectPtr<UObject>& Blocker) { return Blocker.Get() == Source; }))
	{
		return;   // déjà bloqueur
	}

	EmissionBlockers.Add(TWeakObjectPtr<UObject>(Source));

	// Un acteur détruit (statue tuée par le ver ou par l'artefact) lève son blocage immédiatement.
	// AddUniqueDynamic : pas de double abonnement si le même acteur est ajouté, retiré puis rajouté.
	if (AActor* SourceActor = Cast<AActor>(Source))
	{
		SourceActor->OnDestroyed.AddUniqueDynamic(this, &UBlueDustTrailComponent::HandleBlockerDestroyed);
	}

	UE_LOG(LogPlagueTaleGO, Log, TEXT("[BlueDust] Émission bloquée par %s"), *GetNameSafe(Source));
	RefreshBlockedState();
}

void UBlueDustTrailComponent::RemoveEmissionBlocker(UObject* Source)
{
	if (AActor* SourceActor = Cast<AActor>(Source))
	{
		SourceActor->OnDestroyed.RemoveDynamic(this, &UBlueDustTrailComponent::HandleBlockerDestroyed);
	}

	const int32 RemovedCount = EmissionBlockers.RemoveAll([Source](const TWeakObjectPtr<UObject>& Blocker)
	{
		return Blocker.Get() == Source;
	});
	if (RemovedCount > 0)
	{
		UE_LOG(LogPlagueTaleGO, Log, TEXT("[BlueDust] Blocage levé : %s"), *GetNameSafe(Source));
	}
	RefreshBlockedState();
}

bool UBlueDustTrailComponent::IsEmissionBlocked() const
{
	// Un bloqueur détruit (pointeur faible invalide) ne compte plus, même avant d'être retiré du tableau
	return EmissionBlockers.ContainsByPredicate([](const TWeakObjectPtr<UObject>& Blocker) { return Blocker.IsValid(); });
}

void UBlueDustTrailComponent::HandleBlockerDestroyed(AActor* DestroyedActor)
{
	UE_LOG(LogPlagueTaleGO, Log, TEXT("[BlueDust] Bloqueur détruit : %s"), *GetNameSafe(DestroyedActor));
	RemoveEmissionBlocker(DestroyedActor);
}

void UBlueDustTrailComponent::RefreshBlockedState()
{
	EmissionBlockers.RemoveAll([](const TWeakObjectPtr<UObject>& Blocker) { return !Blocker.IsValid(); });

	const bool bBlocked = IsEmissionBlocked();
	if (bBlocked == bWasEmissionBlocked)
	{
		return;
	}
	bWasEmissionBlocked = bBlocked;
	UE_LOG(LogPlagueTaleGO, Log, TEXT("[BlueDust] Émission %s"),
		bBlocked ? TEXT("bloquée : plus aucune trace n'est posée") : TEXT("débloquée : la trace reprend au prochain déplacement"));
	OnEmissionBlockedChanged.Broadcast(bBlocked);
}

// ---------------------------------------------------------------------------
// Debug
// ---------------------------------------------------------------------------
void UBlueDustTrailComponent::SetDrawDebug(bool bEnabled)
{
	bDrawDebug = bEnabled;
#if !UE_BUILD_SHIPPING
	SetComponentTickEnabled(bDrawDebug);
#endif
}

void UBlueDustTrailComponent::DumpToLog() const
{
	UE_LOG(LogPlagueTaleGO, Log, TEXT("[BlueDust] %s : %d trace(s), TrailLength = %d, émission %s"),
		*GetNameSafe(GetOwner()), Traces.Num(), TrailLength, IsEmissionBlocked() ? TEXT("BLOQUÉE") : TEXT("active"));

	for (int32 Index = 0; Index < Traces.Num(); ++Index)
	{
		const FBlueDustTrace& Trace = Traces[Index];
		const FIntPoint Coord = Trace.Node ? Trace.Node->GetGridCoord() : FIntPoint::ZeroValue;
		UE_LOG(LogPlagueTaleGO, Log, TEXT("[BlueDust]   N-%d : %s (%d, %d), durée restante %d"),
			Index + 1, *GetNameSafe(Trace.Node), Coord.X, Coord.Y, Trace.RemainingLifetime);
	}
	for (const TWeakObjectPtr<UObject>& Blocker : EmissionBlockers)
	{
		UE_LOG(LogPlagueTaleGO, Log, TEXT("[BlueDust]   Bloqueur : %s%s"),
			*GetNameSafe(Blocker.Get()), Blocker.IsValid() ? TEXT("") : TEXT(" (détruit)"));
	}
}
