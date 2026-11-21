#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BoardValidationTypes.h"
#include "BoardValidationRule.generated.h"

struct FBoardLayout;

/**
 * Base d'une règle de validation de layout (ex. « aucune case isolée », « un seul départ joueur »).
 *
 * Les règles sont ENFICHABLES : FBoardValidator exécute automatiquement toutes les sous-classes
 * C++ non abstraites de cette classe. Ajouter une règle = créer une classe, rien d'autre à modifier.
 * Retirer une règle = supprimer sa classe.
 *
 * Une règle ne fait que LIRE le layout : pas d'acteur, pas de subsystem, pas d'état.
 * C'est son objet par défaut (CDO) qui est utilisé : aucune instance n'est créée.
 *
 * Recette complète : Source/PlagueTaleGOEditor/CLAUDE.md, « Ajouter une règle de validation ».
 */
UCLASS(Abstract)
class PLAGUETALEGO_API UBoardValidationRule : public UObject
{
	GENERATED_BODY()

public:
	// Exécute la règle et ajoute ses messages au résultat
	void Run(const FBoardLayout& Layout, FBoardValidationResult& OutResult) const;

	EBoardValidationSeverity GetSeverity() const { return Severity; }

	// Texte d'aide : ce que vérifie la règle (affiché par le Level Editor)
	const FText& GetDescription() const { return Description; }

protected:
	// À surcharger : analyser le layout et appeler Report() pour chaque problème trouvé
	virtual void Validate(const FBoardLayout& Layout, TArray<FBoardValidationMessage>& OutMessages) const {}

	// Ajoute un message avec la gravité et le nom de cette règle
	void Report(TArray<FBoardValidationMessage>& OutMessages, const FText& Text,
		const TArray<FIntPoint>& Cells = TArray<FIntPoint>()) const;

	// Utilitaires de mise en forme des messages : « (0, 1) », « (0, 1), (2, 3)… »
	static FString CoordToString(FIntPoint Coord);
	static FString CoordsToString(const TArray<FIntPoint>& Coords);

	// À régler dans le constructeur de chaque règle.
	// Error bloque le Submit, Warning informe seulement.
	EBoardValidationSeverity Severity = EBoardValidationSeverity::Error;

	// À régler dans le constructeur de chaque règle
	FText Description;
};
