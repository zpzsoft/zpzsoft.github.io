// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class PathFollowerEditor : ModuleRules
{
	public PathFollowerEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        string EngineDir = Path.GetFullPath(Target.RelativeEnginePath);

        PublicIncludePaths.AddRange(
                   new string[] {
                    "PathFollowerEditor/Public",
                    "Editor/ClassViewer/Public",
                   }
                   );

        PrivateIncludePaths.AddRange(
            new string[] {
                    "PathFollowerEditor/Private",
                    Path.Combine(EngineDir, @"Source/Developer/AssetTools/Private"),
                // ... add other private include paths required here ...
            }
            );
        PrivateIncludePathModuleNames.AddRange(
            new string[] {
					//"AssetTools",
					"AnimGraph",
                    "BlueprintGraph",
                    "GraphEditor",
                    "Kismet",
                // ... add other private include paths required here ...
            }
            );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "PathFollower",
            }
            );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                    "AnimGraph",
                    "AssetTools",
                    "BlueprintGraph",
                    "ClassViewer",
                    "ContentBrowser",
                    "Core",
                    "CoreUObject",
                    "EditorStyle",
                    "EditorWidgets",
                    "Engine",
                    "GraphEditor",
                    "InputCore",
                    "Kismet",
                    "KismetCompiler",
                    "KismetWidgets",
                    "MainFrame",
                    "Projects",
                    "PropertyEditor",
                    "RHI",
                    "Slate",
                    "SlateCore",
                    "SourceControl",
                    "UnrealEd",
                    "WorkspaceMenuStructure",
            }
            );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
            );
    }
}
