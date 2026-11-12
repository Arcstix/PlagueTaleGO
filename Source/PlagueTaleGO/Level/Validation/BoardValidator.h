#pragma once

#include "CoreMinimal.h"
#include "BoardValidationTypes.h"

struct FBoardLayout;
class UBoardValidationRule;

/**
 * Point d'entrée de la validation d'un layout.
 * Utilisé par le Level Editor (avant le Submit) et par UBoardSubsystem (au lancement de la partie).
 */
struct PLAGUETALEGO_API FBoardValidator
{
	// Exécute toutes les règles (toutes les sous-classes C++ non abstraites de UBoardValidationRule)
	static FBoardValidationResult Validate(const FBoardLayout& Layout);

	// Les règles connues, triées par nom de classe (ordre stable des messages)
	static TArray<const UBoardValidationRule*> GetAllRules();

	// Écrit le résultat dans le log : Error → Error, Warning → Warning. Context = d'où vient la validation.
	static void LogResult(const FBoardValidationResult& Result, const FString& Context);
};
