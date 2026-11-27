#include "BlueDustVisual.h"

#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "PlagueTaleGO/Board/BoardNode.h"
#include "PlagueTaleGO/Entities/ABoardPawn.h"

ABlueDustVisual::ABlueDustVisual()
{
	// Tick seulement pendant une animation (construction, déconstruction, fondu)
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Effect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Effect"));
	Effect->SetupAttachment(Root);
	// Démarré par BeginSegment : l'effet ne doit pas partir tout seul au spawn (ni dans l'aperçu du BP)
	Effect->SetAutoActivate(false);
	// Un effet n'intercepte jamais le clic destiné à la case en dessous
	Effect->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// ---------------------------------------------------------------------------
// Pilotage
// ---------------------------------------------------------------------------
void ABlueDustVisual::BeginSegment(const FBlueDustTrace& Trace, ABoardPawn* InOwner)
{
	if (!Trace.Node)
	{
		Destroy();
		return;
	}

	const FVector Height(0.f, 0.f, TrailHeight);
	StartAnchor = Trace.Node->GetActorLocation() + Height;
	ExitAnchor = Trace.ExitNode ? Trace.ExitNode->GetActorLocation() + Height : StartAnchor;

	// Longueur nulle au départ : la fin va suivre le propriétaire
	CurrentStart = StartAnchor;
	CurrentEnd = StartAnchor;
	Alpha = 1.f;
	OwnerPawn = InOwner;
	Phase = EPhase::Growing;
	PhaseElapsed = 0.f;

	if (Effect)
	{
		Effect->Activate(true);
	}
	PushParameters();
	SetActorTickEnabled(true);

	K2_OnTraceShown();
}

void ABlueDustVisual::Retract(EBlueDustRemovalReason Reason, bool bFromExit, float Duration)
{
	if (Phase == EPhase::RetractingStart || Phase == EPhase::RetractingEnd || Phase == EPhase::FadingOut)
	{
		return;
	}

	K2_OnTraceHidden(Reason);

	Phase = bFromExit ? EPhase::RetractingEnd : EPhase::RetractingStart;
	RetractStartFrom = CurrentStart;
	PhaseElapsed = 0.f;
	PhaseDuration = FMath::Max(Duration, 0.f);
	SetActorTickEnabled(true);
}

void ABlueDustVisual::FadeOut(EBlueDustRemovalReason Reason)
{
	if (Phase == EPhase::FadingOut)
	{
		return;
	}

	K2_OnTraceHidden(Reason);
	StartFadeOut();
}

void ABlueDustVisual::SetLifetimeRatio(float Ratio)
{
	LifetimeRatio = FMath::Clamp(Ratio, 0.f, 1.f);
	if (Effect && !LifetimeRatioParameter.IsNone())
	{
		Effect->SetVariableFloat(LifetimeRatioParameter, LifetimeRatio);
	}
}

// ---------------------------------------------------------------------------
// Animation
// ---------------------------------------------------------------------------
void ABlueDustVisual::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	PhaseElapsed += DeltaSeconds;
	const ABoardPawn* OwnerEntity = OwnerPawn.Get();
	const bool bOwnerMoving = OwnerEntity && OwnerEntity->IsMoving();

	switch (Phase)
	{
	case EPhase::Growing:
		if (bOwnerMoving)
		{
			CurrentEnd = GetOwnerPointOnSegment();
		}
		else
		{
			// Le propriétaire est arrivé : le segment est complet
			CurrentEnd = ExitAnchor;
			Phase = EPhase::Idle;
		}
		break;

	case EPhase::RetractingStart:
	{
		const float Progress = PhaseDuration > KINDA_SMALL_NUMBER ? FMath::Clamp(PhaseElapsed / PhaseDuration, 0.f, 1.f) : 1.f;
		CurrentStart = FMath::Lerp(RetractStartFrom, CurrentEnd, Progress);
		if (Progress >= 1.f)
		{
			StartFadeOut();
		}
		break;
	}

	case EPhase::RetractingEnd:
		if (bOwnerMoving)
		{
			// Le propriétaire revient vers le début : la fin le suit, sans jamais dépasser le début
			CurrentEnd = GetOwnerPointOnSegment();
		}
		else
		{
			CurrentEnd = CurrentStart;
			StartFadeOut();
		}
		break;

	case EPhase::FadingOut:
		Alpha = 1.f - FMath::Clamp(PhaseElapsed / FMath::Max(FadeDuration, KINDA_SMALL_NUMBER), 0.f, 1.f);
		if (Alpha <= 0.f)
		{
			Destroy();
			return;
		}
		break;

	default:
		break;
	}

	PushParameters();

	if (Phase == EPhase::Idle || Phase == EPhase::Inactive)
	{
		SetActorTickEnabled(false);
	}
}

FVector ABlueDustVisual::GetOwnerPointOnSegment() const
{
	const ABoardPawn* OwnerEntity = OwnerPawn.Get();
	const FVector Segment = ExitAnchor - StartAnchor;
	const float LengthSquared = Segment.SizeSquared();
	if (!OwnerEntity || LengthSquared < KINDA_SMALL_NUMBER)
	{
		return StartAnchor;
	}

	// Projection de la position du propriétaire sur le segment, bornée à ses extrémités
	const float T = FMath::Clamp(FVector::DotProduct(OwnerEntity->GetActorLocation() - StartAnchor, Segment) / LengthSquared, 0.f, 1.f);
	return StartAnchor + Segment * T;
}

void ABlueDustVisual::StartFadeOut()
{
	Phase = EPhase::FadingOut;
	PhaseElapsed = 0.f;

	// Plus de nouvelles particules ; celles qui restent s'effacent avec TrailAlpha
	if (Effect)
	{
		Effect->Deactivate();
	}
	SetActorTickEnabled(true);
}

void ABlueDustVisual::PushParameters()
{
	if (!Effect)
	{
		return;
	}

	// SetVariable* vise les paramètres User du Niagara : le nom s'écrit sans le préfixe « User. ».
	// Un paramètre absent du Niagara est simplement ignoré.
	if (!SegmentStartParameter.IsNone())
	{
		Effect->SetVariablePosition(SegmentStartParameter, CurrentStart);
	}
	if (!SegmentEndParameter.IsNone())
	{
		Effect->SetVariablePosition(SegmentEndParameter, CurrentEnd);
	}

	// Le système se place au centre du segment : une zone d'apparition centrée sur lui suit le segment.
	// Émetteurs en espace monde : les particules déjà émises ne bougent pas avec lui.
	Effect->SetWorldLocation((CurrentStart + CurrentEnd) * 0.5f);

	// Boîte alignée sur les axes monde autour du segment. Les liens sont orthogonaux : elle colle au segment.
	// Elle déborde de BoxWidth / 2 à chaque extrémité, ce qui remplit les virages.
	const FVector Delta = CurrentEnd - CurrentStart;
	const FVector Footprint(FMath::Abs(Delta.X) + BoxWidth, FMath::Abs(Delta.Y) + BoxWidth, 0.f);
	if (!BoxSizeParameter.IsNone())
	{
		Effect->SetVariableVec3(BoxSizeParameter, FVector(Footprint.X, Footprint.Y, BoxThickness));
	}
	if (!MaskSizeParameter.IsNone())
	{
		// Boîte centrée sur le système (hauteur de la traînée) : 2 × MaskHeight pour couvrir MaskHeight au-dessus
		Effect->SetVariableVec3(MaskSizeParameter, FVector(Footprint.X, Footprint.Y, 2.f * MaskHeight));
	}
	if (!SegmentLengthParameter.IsNone())
	{
		Effect->SetVariableFloat(SegmentLengthParameter, Delta.Size());
	}
	if (!AlphaParameter.IsNone())
	{
		Effect->SetVariableFloat(AlphaParameter, Alpha);
	}
}
