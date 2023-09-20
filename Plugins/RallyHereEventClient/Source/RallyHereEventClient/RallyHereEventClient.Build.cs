// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;

public class RallyHereEventClient : ModuleRules
{

    public RallyHereEventClient(ReadOnlyTargetRules Target)
        : base(Target)
    {
		PCHUsage = PCHUsageMode.UseSharedPCHs;
        PrivatePCHHeaderFile = "Private/RallyHereEventClientModule.h";

        PublicIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "Public"),
            }
        );
		
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
			"HTTP",
			"Json",
			"JsonUtilities",
        });
    }
}
