using UnrealBuildTool;

public class RallyHereStartEditor : ModuleRules
{
	public RallyHereStartEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		bTreatAsEngineModule = false;
		bEnableUndefinedIdentifierWarnings = false;
		bAllowConfidentialPlatformDefines = true;
		OptimizeCode = CodeOptimization.InNonDebugBuilds;
		PCHUsage = PCHUsageMode.UseSharedPCHs;
		PrivatePCHHeaderFile = "Public/RallyHereStartEditor.h";

		PrivateIncludePathModuleNames.AddRange(
		new string[] {
				"AssetTools",
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"UnrealEd",
				"RallyHereIntegration",
				"DeveloperSettings",
				"GameplayTags",
				"RallyHereStart",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"AssetRegistry",
				"Slate",
				"SlateCore",
			}
		);

		PrivateIncludePaths.AddRange(
		   new string[] {
				"RallyHereStartEditor/Public",
				"RallyHereStartEditor/Private",
		   }
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
				"AssetTools",
			}
		);
	}
}