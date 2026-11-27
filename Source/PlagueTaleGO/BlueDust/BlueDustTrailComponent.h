#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BlueDustTypes.h"
#include "BlueDustTrailComponent.generated.h"

class ABoardNode;
class ABoardPawn;
class UBoardSubsystem;
class UBlueDustTrailComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlueDustTraceAdded, const FBlueDustTrace&, Trace);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBlueDustTraceRemoved, const FBlueDustTrace&, Trace, EBlueDustRemovalReason, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlueDustTrailUpdated, UBlueDustTrailComponent*, Trail);

/**
 * Trace de poussière bleue laissée derrière son propriétaire (le joueur).
 *
 * Responsabilités : SEULE source de vérité des traces (quelles cases, combien de temps encore),
 * et diffusion de chaque changement. Ne change jamais l'occupation du plateau.
 *
 * Règles :
 * - à chaque déplacement du propriétaire (UBoardSubsystem::OnPawnMoved, au début de l'animation) :
 *   1. chaque trace perd 1 de durée de vie ; à 0 elle disparaît (Expired) ;
 *   2. si le propriétaire arrive sur une case tracée, cette trace disparaît (OwnerReturned) ;
 *   3. une trace de durée TrailLength est posée sur la case quittée ;
 * - une seule trace au maximum par case ;
 * - aucune trace au début du niveau (le spawn n'est pas un déplacement).
 * Avec TrailLength = 2, les traces sont donc sur N-1 et N-2.
 *
 * Mécanisme UE : un UActorComponent est un « morceau de comportement » ajouté à un acteur,
 * comme un nœud enfant avec son script dans une scène Godot. Ses réglages apparaissent dans le BP
 * du propriétaire en sélectionnant le composant.
 */
UCLASS(ClassGroup = (Board), meta = (BlueprintSpawnableComponent, ToolTip = "Blue dust trail left behind its owner (the player)."))
class PLAGUETALEGO_API UBlueDustTrailComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBlueDustTrailComponent();

	// --- Lecture ---

	// Nombre de déplacements pendant lesquels une trace reste visible
	UFUNCTION(BlueprintPure, Category = "BlueDust")
	int32 GetTrailLength() const { return TrailLength; }

	// Traces actuelles, de la plus récente (index 0, N-1) à la plus ancienne.
	// Renvoyé par copie : un UFUNCTION exposé au Blueprint ne peut pas renvoyer de référence (quelques éléments, coût négligeable).
	UFUNCTION(BlueprintPure, Category = "BlueDust")
	TArray<FBlueDustTrace> GetTraces() const { return Traces; }

	// --- Événements ---

	// Une trace vient d'apparaître (au début du déplacement du propriétaire)
	UPROPERTY(BlueprintAssignable, Category = "BlueDust")
	FOnBlueDustTraceAdded OnTraceAdded;

	// Une trace vient de disparaître, avec la raison
	UPROPERTY(BlueprintAssignable, Category = "BlueDust")
	FOnBlueDustTraceRemoved OnTraceRemoved;

	// Diffusé une fois toutes les modifications d'un déplacement appliquées (pratique pour un visuel qui relie les traces)
	UPROPERTY(BlueprintAssignable, Category = "BlueDust")
	FOnBlueDustTrailUpdated OnTrailUpdated;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Réglages ---

	// Nombre de déplacements pendant lesquels une trace reste (2 = traces sur N-1 et N-2). 0 = aucune trace.
	UPROPERTY(EditDefaultsOnly, Category = "BlueDust", meta = (ClampMin = "0", UIMax = "10",
		ToolTip = "Number of owner moves a trace stays on its cell (2 = traces on the last two cells left). 0 = no trace."))
	int32 TrailLength = 2;

	// --- Debug ---

	// Dessine les traces (sphère + durée de vie restante) dans le viewport. Jamais dans le build Shipping.
	UPROPERTY(EditDefaultsOnly, Category = "BlueDust|Debug", meta = (
		ToolTip = "Draws each trace (sphere + remaining lifetime) in the viewport. Never in Shipping builds."))
	bool bDrawDebug = true;

	UPROPERTY(EditDefaultsOnly, Category = "BlueDust|Debug", meta = (ClampMin = "1.0", EditCondition = "bDrawDebug",
		ToolTip = "Radius of the debug sphere drawn on each trace."))
	float DebugRadius = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "BlueDust|Debug", meta = (EditCondition = "bDrawDebug",
		ToolTip = "Color of the debug spheres."))
	FColor DebugColor = FColor(40, 120, 255);

	// --- État d'exécution ---

	// Traces actuelles, de la plus récente à la plus ancienne (visible en PIE : Shift+F1, puis sélectionner le joueur)
	UPROPERTY(VisibleInstanceOnly, Category = "BlueDust")
	TArray<FBlueDustTrace> Traces;

private:
	// Réponse à UBoardSubsystem::OnPawnMoved : applique les règles si le pion est le propriétaire
	UFUNCTION()
	void HandlePawnMoved(ABoardPawn* Pawn, ABoardNode* FromNode, ABoardNode* ToNode);

	// 1. Retire 1 de durée de vie à chaque trace, supprime celles qui tombent à 0
	void AgeTraces();

	// 3. Pose une trace sur Node (ou la rafraîchit si la case en porte déjà une)
	void LeaveTrace(ABoardNode* Node);

	// Supprime la trace de Node s'il y en a une, et diffuse OnTraceRemoved. Faux si la case n'avait pas de trace.
	bool RemoveTraceInternal(const ABoardNode* Node, EBlueDustRemovalReason Reason);

	// Index de la trace de Node dans Traces, ou INDEX_NONE
	int32 FindTraceIndex(const ABoardNode* Node) const;

	// Plateau auquel on est abonné (gardé pour se désabonner proprement dans EndPlay)
	TWeakObjectPtr<UBoardSubsystem> BoardSubsystem;
};
