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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlueDustEmissionBlockedChanged, bool, bBlocked);

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
 * - aucune trace au début du niveau (le spawn n'est pas un déplacement) ;
 * - tant qu'au moins un « bloqueur » est actif (ex. une statue qui poursuit le joueur), aucune trace n'est posée.
 *   Un bloqueur détruit libère son blocage de lui-même : la trace reprend au déplacement suivant.
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

	UFUNCTION(BlueprintPure, Category = "BlueDust")
	int32 GetTraceCount() const { return Traces.Num(); }

	// --- Requêtes (pour les features qui lisent la trace : détection et poursuite du ver…) ---

	// Vrai si la case porte une trace
	UFUNCTION(BlueprintPure, Category = "BlueDust")
	bool HasTraceAt(const ABoardNode* Node) const { return FindTraceIndex(Node) != INDEX_NONE; }

	// Déplacements restants avant que la trace de la case disparaisse. 0 = pas de trace.
	UFUNCTION(BlueprintPure, Category = "BlueDust")
	int32 GetTraceLifetime(const ABoardNode* Node) const;

	// Traces situées sur une des cases données (ex. zone de détection d'un ennemi),
	// dans l'ordre de la trace : de la plus récente à la plus ancienne. Vide si aucune.
	UFUNCTION(BlueprintPure, Category = "BlueDust")
	TArray<FBlueDustTrace> FindTracesIn(const TArray<ABoardNode*>& Nodes) const;

	/**
	 * Case suivante pour remonter la trace vers le propriétaire, à partir d'une case tracée :
	 * la trace un cran plus récente, ou la case du propriétaire pour la plus récente (N-1).
	 * nullptr si Node ne porte pas de trace.
	 * Attention : la case renvoyée n'est pas forcément voisine (trace retirée au milieu) : passer par FindPath.
	 */
	UFUNCTION(BlueprintPure, Category = "BlueDust")
	ABoardNode* GetNextTowardOwner(const ABoardNode* Node) const;

	// --- Modifications (pour les features qui agissent sur la trace) ---

	// Retire la trace d'une case (raison Removed). Faux si la case n'avait pas de trace.
	UFUNCTION(BlueprintCallable, Category = "BlueDust")
	bool RemoveTraceAt(ABoardNode* Node);

	// Retire toutes les traces (raison Cleared)
	UFUNCTION(BlueprintCallable, Category = "BlueDust")
	void ClearTrail();

	/**
	 * Change la durée de vie des traces (appelé avant BeginPlay par le réglage du niveau, ou par la commande console).
	 * Les traces existantes sont raccourcies si besoin (jamais rallongées) ; 0 retire tout.
	 */
	UFUNCTION(BlueprintCallable, Category = "BlueDust")
	void SetTrailLength(int32 NewLength);

	// --- Blocage (ex. statue qui poursuit le joueur) ---

	/**
	 * Raccourci pour la statue : retire toutes les traces d'un coup (raison Suppressed) puis bloque l'émission
	 * tant que Source existe ou jusqu'à RemoveEmissionBlocker(Source).
	 */
	UFUNCTION(BlueprintCallable, Category = "BlueDust")
	void SuppressTrail(UObject* Source);

	/**
	 * Empêche de poser de nouvelles traces tant que Source est actif (les traces existantes continuent de vieillir).
	 * Plusieurs bloqueurs peuvent se cumuler (plusieurs statues). Si Source est un acteur, sa destruction le retire automatiquement.
	 */
	UFUNCTION(BlueprintCallable, Category = "BlueDust")
	void AddEmissionBlocker(UObject* Source);

	// Retire le blocage posé par Source (sans effet s'il n'en avait pas)
	UFUNCTION(BlueprintCallable, Category = "BlueDust")
	void RemoveEmissionBlocker(UObject* Source);

	// Vrai si au moins un bloqueur encore vivant empêche de poser des traces
	UFUNCTION(BlueprintPure, Category = "BlueDust")
	bool IsEmissionBlocked() const;

	// --- Debug (commandes console BlueDust.*) ---

	// Active ou coupe le dessin de debug en cours de partie (sans effet dans le build Shipping)
	void SetDrawDebug(bool bEnabled);
	bool IsDrawingDebug() const { return bDrawDebug; }

	// Écrit les traces et les bloqueurs dans le log
	void DumpToLog() const;

	// --- Événements ---

	// Une trace vient d'apparaître (au début du déplacement du propriétaire)
	UPROPERTY(BlueprintAssignable, Category = "BlueDust")
	FOnBlueDustTraceAdded OnTraceAdded;

	// Une trace vient de disparaître, avec la raison
	UPROPERTY(BlueprintAssignable, Category = "BlueDust")
	FOnBlueDustTraceRemoved OnTraceRemoved;

	// Diffusé une fois un ensemble de modifications appliqué (déplacement, suppression, blocage…) :
	// pratique pour un visuel qui relie les traces entre elles
	UPROPERTY(BlueprintAssignable, Category = "BlueDust")
	FOnBlueDustTrailUpdated OnTrailUpdated;

	// L'émission vient d'être bloquée (true) ou débloquée (false)
	UPROPERTY(BlueprintAssignable, Category = "BlueDust")
	FOnBlueDustEmissionBlockedChanged OnEmissionBlockedChanged;

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

	// Objets qui bloquent l'émission. Pointeurs faibles : un bloqueur détruit ne bloque plus, et n'est pas gardé en vie.
	UPROPERTY(VisibleInstanceOnly, Category = "BlueDust")
	TArray<TWeakObjectPtr<UObject>> EmissionBlockers;

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

	// Supprime toutes les traces avec la raison donnée. Faux s'il n'y en avait aucune.
	bool ClearTrailInternal(EBlueDustRemovalReason Reason);

	// Index de la trace de Node dans Traces, ou INDEX_NONE
	int32 FindTraceIndex(const ABoardNode* Node) const;

	// Un acteur bloqueur vient d'être détruit (AActor::OnDestroyed) : son blocage disparaît
	UFUNCTION()
	void HandleBlockerDestroyed(AActor* DestroyedActor);

	// Retire les bloqueurs détruits, puis diffuse OnEmissionBlockedChanged si l'état a changé
	void RefreshBlockedState();

	// Dernier état diffusé par OnEmissionBlockedChanged
	bool bWasEmissionBlocked = false;

	// Plateau auquel on est abonné (gardé pour se désabonner proprement dans EndPlay)
	TWeakObjectPtr<UBoardSubsystem> BoardSubsystem;
};
