// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Bristlecone : ModuleRules
{
	public Bristlecone(ReadOnlyTargetRules Target) : base(Target)
	{
		// Get the engine path. Ends with "Engine/"
		string engine_path = EngineDirectory;
		// Now get the base of UE4's modules dir (could also be Developer, Editor, ThirdParty)
		string src_path = engine_path + "\\Source\\Runtime\\";

		//Don't do this. We need it to avoid having to either patch the engine or rebuild most of sockets or use pointer arithmatic and void*
		PrivateIncludePaths.Add(src_path + "Sockets\\Private\\BSDSockets\\");
		PrivateIncludePaths.Add(src_path + "Sockets\\Private\\");
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicAdditionalLibraries.Add("qwave.lib"); // this will need to be fixed. god.


		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Networking",
			"Sockets",
			"DeveloperSettings",
			"GameInputBase"
		});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Engine",
				"InputCore",
				"Networking",
				"Sockets",
				"Sockets",
				"NetCommon"
			});


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
