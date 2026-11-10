#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Level/Validation/BoardValidationRule.h"
#include "ExitReachableRule.generated.h"

/**
 * Avertissement : aucune sortie n'est atteignable depuis le départ joueur en suivant les liens
 * et en évitant les cases bloquées. Ne tient pas compte des ennemis ni des objets.
 * Ne s'exécute que s'il y a exactement un départ joueur (sinon d'autres règles le signalent déjà).
 */
UCLASS()
class PLAGUETALEGO_API UExitReachableRule : public UBoardValidationRule
{
	GENERATED_BODY()

public:
	UExitReachableRule();

protected:
	virtual void Validate(const FBoardLayout& Layout, TArray<FBoardValidationMessage>& OutMessages) const override;
};
