#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Level/Validation/BoardValidationRule.h"
#include "ExitRule.generated.h"

/**
 * Erreur : le niveau doit contenir au moins une case de sortie
 * (type de case avec le comportement Board.Node.Exit).
 */
UCLASS()
class PLAGUETALEGO_API UExitRule : public UBoardValidationRule
{
	GENERATED_BODY()

public:
	UExitRule();

protected:
	virtual void Validate(const FBoardLayout& Layout, TArray<FBoardValidationMessage>& OutMessages) const override;
};
