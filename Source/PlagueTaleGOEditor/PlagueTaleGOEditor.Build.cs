using System.IO;
using UnrealBuildTool;

// Module éditeur du projet : Level Editor (mode « Plateau ») et outils associés.
// Type « Editor » dans le .uproject : il n'est jamais compilé dans le jeu (ni en Shipping),
// on peut donc y utiliser librement les API de l'éditeur (UnrealEd, Slate…).
public class PlagueTaleGOEditor : ModuleRules
{
	public PlagueTaleGOEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Permet d'inclure les headers du jeu avec le même chemin que dans le module runtime :
		// #include "PlagueTaleGO/Board/Board.h"
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, ".."));

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"PlagueTaleGO",     // le jeu : ABoard, layout, définitions, validation
			"UnrealEd",         // GEditor, transactions (Ctrl+Z), fonctions d'édition des acteurs
			"AssetRegistry",    // recherche des Data Assets de définition
			"GameplayTags",
			// Mode éditeur « Plateau » (UEdMode) et Interactive Tools Framework (outils du viewport)
			"EditorFramework",
			"InteractiveToolsFramework",
			"EditorInteractiveToolsFramework",
			"Slate",
			"SlateCore",
			"InputCore"
		});
	}
}
