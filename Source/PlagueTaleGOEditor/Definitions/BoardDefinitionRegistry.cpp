#include "BoardDefinitionRegistry.h"
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
