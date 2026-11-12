#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Level/Validation/BoardValidationRule.h"
#include "MissingDefinitionRule.generated.h"

/**
 * Erreur : une case, une entité ou un objet n'a pas de type, ou son type est incomplet
 * (ex. Data Asset supprimé, PawnClass non renseignée).
 */
UCLASS()
class PLAGUETALEGO_API UMissingDefinitionRule : public UBoardValidationRule
{
	GENERATED_BODY()

public:
	UMissingDefinitionRule();

protected:
	virtual void Validate(const FBoardLayout& Layout, TArray<FBoardValidationMessage>& OutMessages) const override;
};
