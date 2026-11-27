#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlueDustTypes.h"
#include "BlueDustVisual.generated.h"

class ABoardPawn;
class USceneComponent;
class UNiagaraComponent;

/**
 * Visuel d'UNE trace de poussière bleue : un SEGMENT de sa case (Trace.Node) vers la case par laquelle le joueur
 * est reparti (Trace.ExitNode). Mis bout à bout, les segments forment une traînée continue du plus ancien
 * nœud tracé jusqu'au joueur. Pur affichage : UBlueDustTrailComponent le crée et lui dit quoi faire.
 *
 * Cycle de vie (piloté par le Tick, actif seulement pendant une animation) :
 * - BeginSegment : construction, la fin du segment suit le joueur pendant son déplacement (tête de la traînée) ;
 * - Retract : déconstruction au rythme du joueur, soit par le début (queue qui avance), soit par la fin
 *   (le joueur revient sur ses pas et « mange » le segment) ; puis fondu ;
 * - FadeOut : effacement sur place (statue, ClearTrail…) ;
 * - destruction automatique à la fin du fondu.
 *
 * Le Niagara se choisit dans le BP enfant (composant Effect > Niagara System Asset). Il travaille en ESPACE MONDE
 * (Local Space décoché) et lit les paramètres User envoyés à chaque frame d'animation (voir § Paramètres).
 * Le composant Effect est replacé au centre du segment à chaque frame : une zone d'apparition (Shape Location)
 * centrée sur le système suit donc le segment sans aucun décalage à régler dans le Niagara.
 * Équivalent Godot : une scène d'effet pilotée par le script du joueur, qui met à jour ses shader params.
 */
UCLASS(Blueprintable, NotPlaceable, meta = (ToolTip = "Visual of one blue dust trace: a segment from its cell to the next cell of the player's path (Niagara), built and unbuilt over time. Spawned and driven by the player's BlueDustTrail component."))
class PLAGUETALEGO_API ABlueDustVisual : public AActor
{
	GENERATED_BODY()

public:
	ABlueDustVisual();

	// --- Pilotage par UBlueDustTrailComponent ---

	// Construit le segment Trace.Node → Trace.ExitNode. Sa fin suit InOwner tant qu'il se déplace (tête de la traînée).
	void BeginSegment(const FBlueDustTrace& Trace, ABoardPawn* InOwner);

	/**
	 * Défait le segment, puis fondu et destruction.
	 * bFromExit vrai : la fin recule vers la case en suivant le propriétaire (il revient sur ses pas par ce segment).
	 * bFromExit faux : le début avance vers ExitNode en Duration secondes (queue de la traînée).
	 */
	void Retract(EBlueDustRemovalReason Reason, bool bFromExit, float Duration);

	// Efface le segment sur place en fondu, puis destruction (trace effacée d'un coup : statue, ClearTrail…)
	void FadeOut(EBlueDustRemovalReason Reason);

	// Âge de la trace (1 = neuve, vers 0 = sur le point de disparaître), envoyé au Niagara
	void SetLifetimeRatio(float Ratio);

protected:
	virtual void Tick(float DeltaSeconds) override;

	// --- Événements Blueprint (effets en plus du Niagara, sans code : son, decal…) ---

	// Le segment commence à se construire
	UFUNCTION(BlueprintImplementableEvent, Category = "BlueDust", meta = (DisplayName = "On Trace Shown"))
	void K2_OnTraceShown();

	// Le segment commence à disparaître, avec la raison (ex. Suppressed = une statue l'a effacé)
	UFUNCTION(BlueprintImplementableEvent, Category = "BlueDust", meta = (DisplayName = "On Trace Hidden"))
	void K2_OnTraceHidden(EBlueDustRemovalReason Reason);

	// --- Composants ---

	// Racine : posée au centre de la case de la trace, sans rotation
	UPROPERTY(VisibleAnywhere, Category = "BlueDust")
	TObjectPtr<USceneComponent> Root;

	// Effet de la trace. Niagara choisi dans le BP enfant. Démarré par le code, jamais automatiquement.
	// Déplacé par le code au centre du segment (hauteur Trail Height comprise) : sa position dans le BP est ignorée.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BlueDust")
	TObjectPtr<UNiagaraComponent> Effect;

	// --- Réglages ---

	// Hauteur de la traînée au-dessus de l'origine des cases (cm)
	UPROPERTY(EditDefaultsOnly, Category = "BlueDust", meta = (Units = "cm",
		ToolTip = "Height of the trail above the cells' origin."))
	float TrailHeight = 5.f;

	// Largeur de la zone d'apparition des particules autour du segment (cm). Déborde aussi aux extrémités : couvre les virages.
	UPROPERTY(EditDefaultsOnly, Category = "BlueDust", meta = (ClampMin = "0.0", Units = "cm",
		ToolTip = "Width of the particle spawn box around the segment. It also extends past both ends, which covers the corners."))
	float BoxWidth = 40.f;

	// Épaisseur (Z) de la zone d'apparition des particules (cm)
	UPROPERTY(EditDefaultsOnly, Category = "BlueDust", meta = (ClampMin = "0.0", Units = "cm",
		ToolTip = "Height (Z) of the particle spawn box."))
	float BoxThickness = 2.f;

	// Durée du fondu final (s), après la déconstruction ou pour un effacement d'un coup
	UPROPERTY(EditDefaultsOnly, Category = "BlueDust", meta = (ClampMin = "0.05", Units = "s",
		ToolTip = "Duration of the final fade (after unbuilding, or when the trace is wiped at once). The actor is destroyed at the end."))
	float FadeDuration = 0.6f;

	// --- Paramètres User du Niagara (nom sans le préfixe « User. » ; None = non envoyé) ---

	// Position (monde) : début actuel du segment
	UPROPERTY(EditDefaultsOnly, Category = "BlueDust|Niagara Parameters", meta = (
		ToolTip = "Niagara User parameter (Position, world space): current start of the segment. None = not sent."))
	FName SegmentStartParameter = TEXT("SegmentStart");

	// Position (monde) : fin actuelle du segment (suit le joueur pendant la construction)
	UPROPERTY(EditDefaultsOnly, Category = "BlueDust|Niagara Parameters", meta = (
		ToolTip = "Niagara User parameter (Position, world space): current end of the segment (follows the player while building). None = not sent."))
	FName SegmentEndParameter = TEXT("SegmentEnd");

	// Vector : taille de la zone d'apparition, alignée sur les axes monde (entrée Box Size d'un Shape Location en boîte)
	UPROPERTY(EditDefaultsOnly, Category = "BlueDust|Niagara Parameters", meta = (
		ToolTip = "Niagara User parameter (Vector): world-aligned size of the spawn box (Box Size input of a Box Shape Location). None = not sent."))
	FName BoxSizeParameter = TEXT("SegmentBoxSize");

	// Float : longueur actuelle du segment (cm), ex. pour régler le nombre de particules
	UPROPERTY(EditDefaultsOnly, Category = "BlueDust|Niagara Parameters", meta = (
		ToolTip = "Niagara User parameter (Float): current length of the segment in cm (e.g. to scale the spawn rate). None = not sent."))
	FName SegmentLengthParameter = TEXT("SegmentLength");

	// Float : opacité globale (1 → 0 pendant le fondu final), à multiplier à l'alpha des particules
	UPROPERTY(EditDefaultsOnly, Category = "BlueDust|Niagara Parameters", meta = (
		ToolTip = "Niagara User parameter (Float): global opacity, 1 then down to 0 during the final fade. Multiply the particles' alpha by it. None = not sent."))
	FName AlphaParameter = TEXT("TrailAlpha");

	// Float : durée de vie restante / durée totale (1 = trace neuve). Prévu pour distinguer l'âge plus tard.
	UPROPERTY(EditDefaultsOnly, Category = "BlueDust|Niagara Parameters", meta = (
		ToolTip = "Niagara User parameter (Float): remaining lifetime / trail length (1 = new trace). None = not sent."))
	FName LifetimeRatioParameter = TEXT("LifetimeRatio");

private:
	enum class EPhase : uint8
	{
		Inactive,
		Growing,			// la fin suit le propriétaire
		Idle,				// segment complet, Tick coupé
		RetractingStart,	// le début avance vers la fin (queue)
		RetractingEnd,		// la fin recule vers le début en suivant le propriétaire
		FadingOut			// fondu final, puis destruction
	};

	// Point du segment complet le plus proche du propriétaire (sa position projetée sur Début → ExitNode)
	FVector GetOwnerPointOnSegment() const;

	// Passe au fondu final : plus de nouvelles particules, opacité qui baisse, puis destruction
	void StartFadeOut();

	// Envoie la géométrie actuelle et l'opacité au Niagara
	void PushParameters();

	TWeakObjectPtr<ABoardPawn> OwnerPawn;

	// Extrémités du segment complet (monde, hauteur comprise)
	FVector StartAnchor = FVector::ZeroVector;
	FVector ExitAnchor = FVector::ZeroVector;

	// Extrémités actuellement affichées
	FVector CurrentStart = FVector::ZeroVector;
	FVector CurrentEnd = FVector::ZeroVector;

	// Début du segment au moment où la queue a commencé à reculer
	FVector RetractStartFrom = FVector::ZeroVector;

	EPhase Phase = EPhase::Inactive;
	float PhaseElapsed = 0.f;
	float PhaseDuration = 0.f;
	float Alpha = 1.f;
	float LifetimeRatio = 1.f;
};
