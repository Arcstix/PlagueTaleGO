// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class PlagueTaleGOEditorTarget : TargetRules
{
	public PlagueTaleGOEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("PlagueTaleGO");
		// Module du Level Editor : compilé uniquement pour l'éditeur, jamais dans le jeu
		ExtraModuleNames.Add("PlagueTaleGOEditor");
	}
}
