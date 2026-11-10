#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Level/Validation/BoardValidationRule.h"
#include "IsolatedNodeRule.generated.h"

/**
 * Erreur : une case n'a aucun lien. Avec des liens explicites, une telle case est inaccessible.
 */
UCLASS()
class PLAGUETALEGO_API UIsolatedNodeRule : public UBoardValidationRule
{
	GENERATED_BODY()

public:
	UIsolatedNodeRule();

protected:
	virtual void Validate(const FBoardLayout& Layout, TArray<FBoardValidationMessage>& OutMessages) const override;
};
