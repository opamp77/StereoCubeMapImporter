// Some copyright should be here...

using UnrealBuildTool;

public class StereoCubeMapImporter : ModuleRules
{
	public StereoCubeMapImporter(TargetInfo Target)
	{
		
		PublicIncludePaths.AddRange(
			new string[] {
				"StereoCubeMapImporter/Public",
				"Developer/DesktopPlatform/Public",
                "Developer/ImageWrapper/Public/Interfaces/",
                "Runtime/AssetRegistry/Public/",
                "Editor/ContentBrowser/Public/"
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"StereoCubeMapImporter/Private",
				
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"DesktopPlatform",
                "ImageWrapper",
                "AssetRegistry",
                "RenderCore",
                "ContentBrowser"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"UnrealEd",
				"LevelEditor",
				"CoreUObject", "Engine", "Slate", "SlateCore"
				
				// ... add private dependencies that you statically link with here ...	
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
