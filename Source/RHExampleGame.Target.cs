// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class RHExampleGameTarget : TargetRules
{
    public RHExampleGameTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;

        DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Mac || Target.Platform == UnrealTargetPlatform.Linux)
        {
			UndecoratedConfiguration = UnrealTargetConfiguration.Shipping;
			bUsesSteam = true;
        }

        // Do not allow generated inis except on desktop / mobile platforms
        if (
            Target.Platform == UnrealTargetPlatform.Win64 ||
            Target.Platform == UnrealTargetPlatform.Mac ||
            Target.Platform == UnrealTargetPlatform.IOS ||
            Target.Platform == UnrealTargetPlatform.Android ||
            Target.Platform == UnrealTargetPlatform.Linux )
        {
            bAllowGeneratedIniWhenCooked = true;
        }
        else
        {
            bAllowGeneratedIniWhenCooked = false;
        }

        // Do not allow non UFS inis when cooked
        bAllowNonUFSIniWhenCooked = false;

        ExtraModuleNames.Add("RHExampleGame");
    }
}
