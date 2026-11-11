#include "BoardDefinitionRegistry.h"
#include "PlagueTaleGOEditor/PlagueTaleGOEditor.h"
#include "PlagueTaleGO/Level/Definitions/BoardNodeType.h"
#include "AssetRegistry/AssetRegistryModule.h"

namespace
{
	// Dossier ignoré : prototypes, jamais utilisés par un niveau de production
	const TCHAR* ExcludedRoot = TEXT("/Game/Sandbox/");
}

TArray<UBoardDefinition*> FBoardDefinitionRegistry::GetAll(const UClass* DefinitionClass)
{
	TArray<UBoardDefinition*> Result;
	if (!DefinitionClass)
	{
		return Result;
	}

	// L'Asset Registry connaît tous les assets du projet sans les charger.
	// Équivalent Godot : parcourir res:// avec EditorFileSystem.
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	TArray<FAssetData> Assets;
	AssetRegistry.GetAssetsByClass(DefinitionClass->GetClassPathName(), Assets, /*bSearchSubClasses*/ true);

	for (const FAssetData& Asset : Assets)
	{
		if (Asset.PackageName.ToString().StartsWith(ExcludedRoot))
		{
			continue;
		}
		if (UBoardDefinition* Definition = Cast<UBoardDefinition>(Asset.GetAsset()))
		{
			Result.Add(Definition);
		}
	}

	Result.Sort([](const UBoardDefinition& A, const UBoardDefinition& B)
	{
		if (A.SortOrder != B.SortOrder)
		{
			return A.SortOrder < B.SortOrder;
		}
		return A.GetDisplayText().CompareTo(B.GetDisplayText()) < 0;
	});
	return Result;
}

UBoardNodeType* FBoardDefinitionRegistry::GetDefaultNodeType()
{
	const TArray<UBoardNodeType*> Types = GetAll<UBoardNodeType>();

	UBoardNodeType* Found = nullptr;
	int32 Count = 0;
	for (UBoardNodeType* Type : Types)
	{
		if (Type->bIsDefaultType)
		{
			if (!Found)
			{
				Found = Type;
			}
			++Count;
		}
	}

	if (Count > 1)
	{
		UE_LOG(LogBoardEditor, Warning, TEXT("[BoardEditor] %d types de case cochés « Type par défaut » : %s est utilisé."),
			Count, *GetNameSafe(Found));
	}
	if (!Found && Types.Num() > 0)
	{
		Found = Types[0];
		UE_LOG(LogBoardEditor, Warning, TEXT("[BoardEditor] Aucun type de case coché « Type par défaut » : %s est utilisé."),
			*GetNameSafe(Found));
	}
	return Found;
}
