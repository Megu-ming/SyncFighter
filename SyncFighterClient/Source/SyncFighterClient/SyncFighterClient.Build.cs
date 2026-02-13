// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SyncFighterClient : ModuleRules
{
	public SyncFighterClient(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
			"Sockets", "Networking",
			"UMG", "Slate", "SlateCore"
		});
	}
}
