#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Level/Validation/BoardValidationRule.h"
#include "PlayerStartRule.generated.h"

/**
 * Erreur : le niveau doit contenir exactement UN départ joueur
 * (une entité dont le type crée un pion dérivé de APlayerPawn).
 */
UCLASS()
class PLAGUETALEGO_API UPlayerStartRule : public UBoardValidationRule
{
	GENERATED_BODY()

public:
	UPlayerStartRule();

protected:
	virtual void Validate(const FBoardLayout& Layout, TArray<FBoardValidationMessage>& OutMessages) const override;
};
