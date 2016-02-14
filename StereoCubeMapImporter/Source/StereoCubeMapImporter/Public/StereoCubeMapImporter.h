// Some copyright should be here...

#pragma once

#include "ModuleManager.h"
#include "DesktopPlatformModule.h"
#include "ImageWrapper.h"


class FToolBarBuilder;
class FMenuBuilder;
class FDesktopPlatformModule;
class IImageWrapperModule;

class FStereoCubeMapImporterModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	void PluginButtonClicked();
	
private:

	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

	bool GetCubeFaceData(const TArray<uint8>* RawData, uint8 FaceIndex, uint32 MipSize, uint32 SizeX, TArray<uint8>* OutData);
	void MirrorImageData(TArray<uint8>* Data, uint32 SizeX);
	void RotateImageDataAntiClockwise90(TArray<uint8>* Data, uint32 SizeX);
	void RotateImageDataClockwise90(TArray<uint8>* Data, uint32 SizeX);
	void RotateImageData180(TArray<uint8>* Data, uint32 SizeX);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};