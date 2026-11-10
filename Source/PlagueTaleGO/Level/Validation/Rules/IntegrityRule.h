#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Level/Validation/BoardValidationRule.h"
#include "IntegrityRule.generated.h"

/**
 * Erreur : données incohérentes dans le layout.
 * - deux cases avec les mêmes coordonnées ;
 * - un lien vers une case absente, entre deux cases non voisines ou d'une case vers elle-même.
 * Le Level Editor ne produit pas ces cas : cette règle protège contre une donnée corrompue ou éditée à la main.
 */
UCLASS()
class PLAGUETALEGO_API UIntegrityRule : public UBoardValidationRule
{
	GENERATED_BODY()

public:
	UIntegrityRule();

protected:
	virtual void Validate(const FBoardLayout& Layout, TArray<FBoardValidationMessage>& OutMessages) const override;
};
