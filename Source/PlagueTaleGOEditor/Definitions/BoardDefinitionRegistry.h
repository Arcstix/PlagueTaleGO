#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Level/Definitions/BoardDefinition.h"

/**
 * Trouve tous les Data Assets de définition du projet (types de cases, d'entités, d'objets).
 *
 * C'est ce qui rend le Level Editor évolutif : sa palette est construite à partir des assets
 * qui existent dans Content/, au moment où on l'ouvre. Ajouter un type = créer un asset.
 *
 * Les assets de Content/Sandbox/ sont ignorés (règle du projet : jamais référencés par un niveau).
 */
struct FBoardDefinitionRegistry
{
	// Tous les assets de cette classe (sous-classes comprises), triés par SortOrder puis par nom affiché.
	// Charge les assets trouvés (ils sont petits).
	static TArray<UBoardDefinition*> GetAll(const UClass* DefinitionClass);

	// Version typée : FBoardDefinitionRegistry::GetAll<UBoardNodeType>()
	template <typename T>
	static TArray<T*> GetAll()
	{
		TArray<T*> Result;
		for (UBoardDefinition* Definition : GetAll(T::StaticClass()))
		{
			if (T* Typed = Cast<T>(Definition))
			{
				Result.Add(Typed);
			}
		}
		return Result;
	}
};
