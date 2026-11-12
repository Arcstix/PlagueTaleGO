#include "BoardValidator.h"
#include "BoardValidationRule.h"
#include "PlagueTaleGO/PlagueTaleGO.h"
#include "PlagueTaleGO/Level/BoardLayout.h"
#include "UObject/UObjectHash.h"   // GetDerivedClasses

FBoardValidationResult FBoardValidator::Validate(const FBoardLayout& Layout)
{
	FBoardValidationResult Result;
	for (const UBoardValidationRule* Rule : GetAllRules())
	{
		Rule->Run(Layout, Result);
	}
	return Result;
}

TArray<const UBoardValidationRule*> FBoardValidator::GetAllRules()
{
	// Toutes les classes chargées qui dérivent de UBoardValidationRule.
	// Les classes C++ sont toutes chargées au démarrage : une nouvelle règle est trouvée sans l'enregistrer nulle part.
	TArray<UClass*> Classes;
	GetDerivedClasses(UBoardValidationRule::StaticClass(), Classes, /*bRecursive*/ true);

	Classes.RemoveAll([](const UClass* Class)
	{
		// Abstract : classe de base ; Deprecated / NewerVersionExists : anciennes versions après une recompilation à chaud
		return !Class || Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists);
	});

	Classes.Sort([](const UClass& A, const UClass& B)
	{
		return A.GetName() < B.GetName();
	});

	TArray<const UBoardValidationRule*> Rules;
	for (UClass* Class : Classes)
	{
		if (const UBoardValidationRule* Rule = Class->GetDefaultObject<UBoardValidationRule>())
		{
			Rules.Add(Rule);
		}
	}
	return Rules;
}

void FBoardValidator::LogResult(const FBoardValidationResult& Result, const FString& Context)
{
	if (Result.Messages.Num() == 0)
	{
		UE_LOG(LogPlagueTaleGO, Log, TEXT("[Validation] %s : layout valide."), *Context);
		return;
	}

	UE_LOG(LogPlagueTaleGO, Log, TEXT("[Validation] %s : %d erreur(s), %d avertissement(s)."),
		*Context, Result.CountErrors(), Result.CountWarnings());

	for (const FBoardValidationMessage& Message : Result.Messages)
	{
		if (Message.Severity == EBoardValidationSeverity::Error)
		{
			UE_LOG(LogPlagueTaleGO, Error, TEXT("[Validation] %s"), *Message.Text.ToString());
		}
		else
		{
			UE_LOG(LogPlagueTaleGO, Warning, TEXT("[Validation] %s"), *Message.Text.ToString());
		}
	}
}
