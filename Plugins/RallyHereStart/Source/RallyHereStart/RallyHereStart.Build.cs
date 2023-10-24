// Copyright 2016-2017 Hi-Rez Studios, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class RallyHereStart : ModuleRules
{
	public RallyHereStart(ReadOnlyTargetRules Target)
		: base(Target)
	{
		PCHUsage = PCHUsageMode.UseSharedPCHs;
		PrivatePCHHeaderFile = "Public/RallyHereStart.h";
		bLegacyPublicIncludePaths = false;

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreOnline",
				"CoreUObject",
				"RenderCore",
				"Engine",
				"InputCore",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
				"RallyHereAPI",
				"RallyHereEventClient",
				"RallyHereIntegration",
				"HTTP",
				"RHI",
				"UMG",
				"Json",
				"JsonUtilities",
				"LevelSequence",
				"MoviePlayer",
				"MovieScene",
				"AssetRegistry",
				"GameplayTags",
				"MediaAssets",
				"MediaUtils",
				"ApplicationCore",
				"DeveloperSettings",
				"NetCore",
				"EnhancedInput",
				"CommonUI"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"AIModule",
				"AssetRegistry",
				"PerfCounters",
				"SlateCore",
				"Slate",
				"Sockets"
			}
		);

		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Public"),
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
				"RallyHereStart/Public",
				"RallyHereStart/Private",
			}
		);

		bool bPlatformSupportsWebAuth = Target.Platform == UnrealTargetPlatform.IOS;
		if (bPlatformSupportsWebAuth)
		{
			PrivateDependencyModuleNames.Add("WebAuth");
			PrivateDefinitions.Add("RALLYSTART_USE_WEBAUTH_MODULE=1");
		}
		else
		{
			PrivateDefinitions.Add("RALLYSTART_USE_WEBAUTH_MODULE=0");
		}
	}
}
