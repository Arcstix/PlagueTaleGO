#pragma once

#include "CoreMinimal.h"
#include "BoardValidationTypes.generated.h"

// Gravité d'un problème détecté dans un layout
UENUM(BlueprintType)
enum class EBoardValidationSeverity : uint8
{
	// Information : le Submit reste possible
	Warning	UMETA(DisplayName = "Warning"),

	// Bloque le Submit (sauf si l'option « Ignore Errors » du Level Editor est cochée)
	Error	UMETA(DisplayName = "Error")
};

// Un problème détecté par une règle de validation
USTRUCT(BlueprintType)
struct PLAGUETALEGO_API FBoardValidationMessage
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
	EBoardValidationSeverity Severity = EBoardValidationSeverity::Error;

	// Message lisible par un LD : quoi, où, et si possible comment corriger
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
	FText Text;

	// Cases concernées (le Level Editor les met en évidence). Peut être vide.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
	TArray<FIntPoint> Cells;

	// Nom de la classe de la règle qui a produit le message (debug)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
	FName RuleName;
};

// Résultat complet d'une validation
USTRUCT(BlueprintType)
struct PLAGUETALEGO_API FBoardValidationResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
	TArray<FBoardValidationMessage> Messages;

	bool HasErrors() const { return CountErrors() > 0; }

	int32 CountErrors() const { return Count(EBoardValidationSeverity::Error); }
	int32 CountWarnings() const { return Count(EBoardValidationSeverity::Warning); }

	int32 Count(EBoardValidationSeverity Severity) const
	{
		int32 Result = 0;
		for (const FBoardValidationMessage& Message : Messages)
		{
			if (Message.Severity == Severity)
			{
				++Result;
			}
		}
		return Result;
	}
};
