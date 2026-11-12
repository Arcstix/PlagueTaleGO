#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Level/Validation/BoardValidationRule.h"
#include "ConnectedGraphRule.generated.h"

/**
 * Erreur : toutes les cases doivent former un seul ensemble relié (pas d'« îlot »).
 * On ne tient compte que des liens, pas des cases bloquées.
 */
UCLASS()
class PLAGUETALEGO_API UConnectedGraphRule : public UBoardValidationRule
{
	GENERATED_BODY()

public:
	UConnectedGraphRule();

protected:
	virtual void Validate(const FBoardLayout& Layout, TArray<FBoardValidationMessage>& OutMessages) const override;
};
