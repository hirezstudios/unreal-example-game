// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class RHExampleGameEditorTarget : TargetRules
{
	public RHExampleGameEditorTarget(TargetInfo Target) : base(Target)
    {
		Type = TargetType.Editor;
        UndecoratedConfiguration = UnrealTargetConfiguration.Development;

        DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.Add("RHExampleGame");
    }
}
