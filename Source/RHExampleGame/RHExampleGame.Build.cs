// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RHExampleGame : ModuleRules
{
	public RHExampleGame(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseSharedPCHs;
        PrivatePCHHeaderFile = "Public/RHExampleGame.h";
        bLegacyPublicIncludePaths = false;

		PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "RenderCore",
                "InputCore",
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "RallyHereAPI",
                "RallyHereIntegration",  
                "GameplayTags",
                "LevelSequence",
                "GameplayTasks",
                "MoviePlayer",
                "AIModule",
                "Json",
                "JsonUtilities",
				"HTTP",
				"RallyHereStart"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "NavigationSystem",
                "MoviePlayer",
                "MovieScene",
                "AssetRegistry",
                "Slate",
                "SlateCore",
                "UMG",
				"RHI",
				"ImageWrapper",
                "MediaAssets",
            }
        );

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemHotfix");

        if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Mac)
        {
			DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
			DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteamV2");
            DynamicallyLoadedModuleNames.Add("OnlineSubsystemEOS");
        }

        if (Target.bBuildEditor == true)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
            PrivateDependencyModuleNames.Add("RHI");
            PrivateDependencyModuleNames.Add("RenderCore");
            PrivateDependencyModuleNames.Add("SlateCore");
            PrivateDependencyModuleNames.Add("EngineSettings");
        }

        PrivateIncludePaths.AddRange(new string[] {
            "RHExampleGame/Public",
			"RHExampleGame/Private"
		});
    }
}
