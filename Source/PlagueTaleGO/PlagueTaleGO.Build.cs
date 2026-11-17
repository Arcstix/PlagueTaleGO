// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PlagueTaleGO : ModuleRules
{
	public PlagueTaleGO(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		// GameplayTags : comportements des types de cases (Board.Node.*), utilisés dans des headers publics
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "GameplayTags" });

		PrivateDependencyModuleNames.AddRange(new string[] { });
		
		if (Target.bBuildEditor) { PrivateDependencyModuleNames.Add("UnrealEd"); }

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
