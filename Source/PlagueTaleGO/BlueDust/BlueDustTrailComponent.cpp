#include "BlueDustTrailComponent.h"
#include "BlueDustVisual.h"

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

	// Propriétaire détruit en cours de partie : ses visuels disparaissent avec lui.
	// Fin de niveau : le monde détruit déjà tous les acteurs, on oublie seulement les pointeurs.
	if (EndPlayReason == EEndPlayReason::Destroyed)
	{
		DestroyAllVisuals();
	}
	TraceVisuals.Reset();
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
		if (Trace.ExitNode)
		{
			// Segment de la trace : vers la case par laquelle le propriétaire est reparti
			const FVector ExitLocation = Trace.ExitNode->GetActorLocation() + FVector(0.f, 0.f, DebugRadius);
			DrawDebugLine(World, Location, ExitLocation, DebugColor, false, 0.f, 0, 3.f);
		}
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

	CurrentMoveFrom = FromNode;

	// L'ordre compte : vieillir d'abord, sinon la nouvelle trace perdrait 1 dès sa pose
	AgeTraces();
	RemoveTraceInternal(ToNode, EBlueDustRemovalReason::OwnerReturned);
	if (!IsEmissionBlocked())
	{
		LeaveTrace(FromNode, ToNode);
	}

	CurrentMoveFrom = nullptr;
	NotifyTrailUpdated();
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
			NotifyTraceRemoved(Expired, EBlueDustRemovalReason::Expired);
		}
	}
}

void UBlueDustTrailComponent::LeaveTrace(ABoardNode* Node, ABoardNode* ExitNode)
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
		Refreshed.ExitNode = ExitNode;
		Traces.RemoveAt(Existing);
		Traces.Insert(Refreshed, 0);
		UE_LOG(LogPlagueTaleGO, Verbose, TEXT("[BlueDust] Trace rafraîchie sur %s"), *GetNameSafe(Node));

		// Le segment a changé de direction : l'ancien s'efface, le nouveau se construit
		TWeakObjectPtr<ABlueDustVisual> OldVisual;
		if (TraceVisuals.RemoveAndCopyValue(Node, OldVisual) && OldVisual.IsValid())
		{
			OldVisual->FadeOut(EBlueDustRemovalReason::Removed);
		}
		SpawnVisual(Refreshed);
		return;
	}

	FBlueDustTrace NewTrace;
	NewTrace.Node = Node;
	NewTrace.ExitNode = ExitNode;
	NewTrace.RemainingLifetime = TrailLength;
	Traces.Insert(NewTrace, 0);   // index 0 = la plus récente (N-1)

	UE_LOG(LogPlagueTaleGO, Verbose, TEXT("[BlueDust] Trace posée sur %s (durée %d)"), *GetNameSafe(Node), TrailLength);
	NotifyTraceAdded(NewTrace);
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
	NotifyTraceRemoved(Removed, Reason);
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
		NotifyTraceRemoved(Trace, Reason);
	}
	UE_LOG(LogPlagueTaleGO, Log, TEXT("[BlueDust] %d trace(s) retirée(s) (%s)"), Removed.Num(), *UEnum::GetValueAsString(Reason));
	return true;
}

// ---------------------------------------------------------------------------
// Diffusion et visuels
// ---------------------------------------------------------------------------
void UBlueDustTrailComponent::NotifyTraceAdded(const FBlueDustTrace& Trace)
{
	SpawnVisual(Trace);
	OnTraceAdded.Broadcast(Trace);
}

void UBlueDustTrailComponent::SpawnVisual(const FBlueDustTrace& Trace)
{
	UWorld* World = GetWorld();
	if (!TraceClass || !World || !Trace.Node)
	{
		return;
	}

	// Un visuel qui disparaît encore sur cette case n'est pas réutilisé : son segment allait ailleurs.
	// Il finit son fondu seul ; le nouveau prend sa place dans TraceVisuals.
	FActorSpawnParameters Params;
	Params.Owner = GetOwner();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	// Sans rotation : le visuel travaille en espace monde (extrémités du segment calculées en coordonnées monde)
	ABlueDustVisual* Visual = World->SpawnActor<ABlueDustVisual>(TraceClass, Trace.Node->GetActorLocation(), FRotator::ZeroRotator, Params);
	if (!Visual)
	{
		return;
	}
	TraceVisuals.Add(Trace.Node, Visual);
	Visual->BeginSegment(Trace, Cast<ABoardPawn>(GetOwner()));
}

void UBlueDustTrailComponent::NotifyTraceRemoved(const FBlueDustTrace& Trace, EBlueDustRemovalReason Reason)
{
	// Le visuel quitte TraceVisuals tout de suite : il se défait puis se détruit seul
	TWeakObjectPtr<ABlueDustVisual> WeakVisual;
	TraceVisuals.RemoveAndCopyValue(Trace.Node, WeakVisual);
	if (ABlueDustVisual* Visual = WeakVisual.Get())
	{
		switch (Reason)
		{
		case EBlueDustRemovalReason::Expired:
			// Queue de la trace : le début du segment avance vers ExitNode, au rythme du propriétaire
			Visual->Retract(Reason, false, GetOwnerMoveDuration());
			break;

		case EBlueDustRemovalReason::OwnerReturned:
			// Propriétaire qui revient par ce segment (ExitNode == case qu'il quitte) : la fin du segment le suit
			// et le segment est « mangé » sous ses pas. Sinon (boucle) : il se défait comme une queue.
			Visual->Retract(Reason, Trace.ExitNode == CurrentMoveFrom, GetOwnerMoveDuration());
			break;

		default:
			// Statue, ClearTrail, RemoveTraceAt : effacé d'un coup, en fondu
			Visual->FadeOut(Reason);
			break;
		}
	}

	OnTraceRemoved.Broadcast(Trace, Reason);
}

float UBlueDustTrailComponent::GetOwnerMoveDuration() const
{
	const ABoardPawn* OwnerPawn = Cast<ABoardPawn>(GetOwner());
	return OwnerPawn ? OwnerPawn->GetMoveDuration() : 0.4f;
}

void UBlueDustTrailComponent::NotifyTrailUpdated()
{
	// Oublie les visuels détruits à la fin de leur fondu
	for (auto It = TraceVisuals.CreateIterator(); It; ++It)
	{
		if (!It->Value.IsValid())
		{
			It.RemoveCurrent();
		}
	}

	// Âge de chaque trace transmis à son visuel (paramètre LifetimeRatio du Niagara : 1 = trace neuve)
	for (const FBlueDustTrace& Trace : Traces)
	{
		if (ABlueDustVisual* Visual = TraceVisuals.FindRef(Trace.Node).Get())
		{
			Visual->SetLifetimeRatio(TrailLength > 0 ? static_cast<float>(Trace.RemainingLifetime) / TrailLength : 0.f);
		}
	}

	OnTrailUpdated.Broadcast(this);
}

void UBlueDustTrailComponent::DestroyAllVisuals()
{
	for (const TPair<TObjectPtr<ABoardNode>, TWeakObjectPtr<ABlueDustVisual>>& Pair : TraceVisuals)
	{
		if (ABlueDustVisual* Visual = Pair.Value.Get())
		{
			Visual->Destroy();
		}
	}
	TraceVisuals.Reset();
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
	return Index != INDEX_NONE ? Traces[Index].ExitNode.Get() : nullptr;
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
	NotifyTrailUpdated();
	return true;
}

void UBlueDustTrailComponent::ClearTrail()
{
	if (ClearTrailInternal(EBlueDustRemovalReason::Cleared))
	{
		NotifyTrailUpdated();
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
		NotifyTrailUpdated();
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
		NotifyTrailUpdated();
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
		UE_LOG(LogPlagueTaleGO, Log, TEXT("[BlueDust]   N-%d : %s (%d, %d), durée restante %d, repart vers %s"),
			Index + 1, *GetNameSafe(Trace.Node), Coord.X, Coord.Y, Trace.RemainingLifetime, *GetNameSafe(Trace.ExitNode));
	}
	for (const TWeakObjectPtr<UObject>& Blocker : EmissionBlockers)
	{
		UE_LOG(LogPlagueTaleGO, Log, TEXT("[BlueDust]   Bloqueur : %s%s"),
			*GetNameSafe(Blocker.Get()), Blocker.IsValid() ? TEXT("") : TEXT(" (détruit)"));
	}
}
