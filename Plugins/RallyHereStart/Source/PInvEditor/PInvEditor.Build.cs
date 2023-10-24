using UnrealBuildTool;

public class PInvEditor : ModuleRules
{
	public PInvEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		bEnforceIWYU = false;
		bTreatAsEngineModule = false;
		bEnableUndefinedIdentifierWarnings = false;
		bAllowConfidentialPlatformDefines = true;
		OptimizeCode = CodeOptimization.InNonDebugBuilds;
		PCHUsage = PCHUsageMode.UseSharedPCHs;
		PrivatePCHHeaderFile = "Public/PInvEditor.h";

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
				"RallyHereStart",
				"DeveloperSettings",
				"GameplayTags"
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
				"PInvEditor/Public",
				"PInvEditor/Private",
		   }
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
				"AssetTools",
			}
		);
	}
}