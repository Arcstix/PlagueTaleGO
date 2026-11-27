// PlayerPawn.h
#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Entities/ABoardPawn.h"
#include "PlagueTaleGO/Board/BoardTypes.h"
#include "APlayerPawn.generated.h"

class ABoardNode;
class UBlueDustTrailComponent;

/**
 * Pion du joueur. Sert à identifier le joueur (Cast<APlayerPawn>) et porte ce qui lui est propre :
 * la trace de poussière bleue (UBlueDustTrailComponent), réglable dans BP_PlayerPawn en sélectionnant le composant.
 */
UCLASS()
class PLAGUETALEGO_API APlayerPawn : public ABoardPawn
{
	GENERATED_BODY()

public:
	APlayerPawn();

	// Trace de poussière bleue laissée derrière le joueur (point d'accès des futures features : ver, statue…)
	UFUNCTION(BlueprintPure, Category = "BlueDust")
	UBlueDustTrailComponent* GetBlueDustTrail() const { return BlueDustTrail; }

protected:
	// Créé en C++ : tous les BP enfants l'ont, sans risque de l'oublier
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BlueDust")
	TObjectPtr<UBlueDustTrailComponent> BlueDustTrail;
};