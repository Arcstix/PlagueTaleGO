#pragma once

#include "CoreMinimal.h"
#include "BlueDustTypes.generated.h"

class ABoardNode;

/**
 * Pourquoi une trace de poussière bleue a disparu.
 * Transmis par UBlueDustTrailComponent::OnTraceRemoved : le visuel, l'audio ou une future feature
 * peuvent réagir différemment (fondu normal, effet de la statue…).
 */
UENUM(BlueprintType)
enum class EBlueDustRemovalReason : uint8
{
	Expired			UMETA(ToolTip = "Lifetime ran out: the owner moved TrailLength times since the trace was left."),
	OwnerReturned	UMETA(ToolTip = "The owner stepped back onto the cell: the owner is always the head of the trail."),
	Suppressed		UMETA(ToolTip = "Wiped by an emission blocker (e.g. a statue chasing the player)."),
	Removed			UMETA(ToolTip = "Removed on purpose by another gameplay feature."),
	Cleared			UMETA(ToolTip = "The whole trail was cleared (debug command, end of game...).")
};

/**
 * Une trace de poussière bleue : une case et le nombre de déplacements du propriétaire avant qu'elle disparaisse.
 * Donnée pure, gérée uniquement par UBlueDustTrailComponent (le visuel ne fait que l'afficher).
 */
USTRUCT(BlueprintType)
struct PLAGUETALEGO_API FBlueDustTrace
{
	GENERATED_BODY()

	// Case qui porte la trace
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "BlueDust")
	TObjectPtr<ABoardNode> Node = nullptr;

	// Déplacements restants avant disparition (TrailLength à la pose, puis -1 à chaque déplacement du propriétaire)
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "BlueDust")
	int32 RemainingLifetime = 0;
};
