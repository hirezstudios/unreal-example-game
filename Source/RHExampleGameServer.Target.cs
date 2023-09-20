// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class RHExampleGameServerTarget : TargetRules
{
    public RHExampleGameServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        UndecoratedConfiguration = UnrealTargetConfiguration.Shipping;

        DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		bUsesSlate = false;

        // Turn on shipping logging
        bUseLoggingInShipping = true;

		// servers should never use generated ini files
        bAllowGeneratedIniWhenCooked = false;

        ExtraModuleNames.Add("RHExampleGame");

        LinkType = TargetLinkType.Monolithic;
    }
}