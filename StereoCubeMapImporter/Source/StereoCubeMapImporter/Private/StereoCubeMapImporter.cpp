// Some copyright should be here...
#if WITH_EDITOR

#include "StereoCubeMapImporterPrivatePCH.h"

#include "SlateBasics.h"
#include "SlateExtras.h"

#include "StereoCubeMapImporterStyle.h"
#include "StereoCubeMapImporterCommands.h"

#include "LevelEditor.h"

#include "AssetRegistryModule.h"
#include "ContentBrowserModule.h"


static const FName StereoCubeMapImporterTabName("Import Stereo Cube");

#define LOCTEXT_NAMESPACE "FStereoCubeMapImporterModule"

void FStereoCubeMapImporterModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FStereoCubeMapImporterStyle::Initialize();
	FStereoCubeMapImporterStyle::ReloadTextures();

	FStereoCubeMapImporterCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FStereoCubeMapImporterCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FStereoCubeMapImporterModule::PluginButtonClicked),
		FCanExecuteAction());

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FStereoCubeMapImporterModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}

	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FStereoCubeMapImporterModule::AddToolbarExtension));

		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}


}

void FStereoCubeMapImporterModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.


	FStereoCubeMapImporterStyle::Shutdown();

	FStereoCubeMapImporterCommands::Unregister();
}

void FStereoCubeMapImporterModule::PluginButtonClicked()
{
	TArray<FString> OutFileNames;
	FDesktopPlatformModule::Get()->OpenFileDialog(nullptr, "Import Stereo CubeMap Image", "", "", "Image Files (*.bmp, *.jpg *.png)|*.bmp;*.jpg;*.jpeg;*.png", 0, OutFileNames);
	if (OutFileNames.Num())
	{
		TArray<uint8> RawFileData;

		if (FFileHelper::LoadFileToArray(RawFileData, *OutFileNames[0]))
		{
			IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
			IImageWrapperPtr ImageWrappers[3] =
			{
				ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG),
				ImageWrapperModule.CreateImageWrapper(EImageFormat::BMP),
				ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG),
			};

			bool wrappersuccess = false;

			for (auto ImageWrapper : ImageWrappers)
			{
				if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
				{
					const TArray<uint8>* RawData = nullptr;
					if (ImageWrapper->GetRaw(ERGBFormat::BGRA/*ImageWrapper->GetFormat()*/, ImageWrapper->GetBitDepth(), RawData))
					{
						TArray<uint8> FaceData;

						uint32 ImageWidth = ImageWrapper->GetWidth();
						uint32 ImageHeight = ImageWrapper->GetHeight();
						//check dimensions and early out if incorrect.
						if (ImageHeight > 2048)
						{
							FString DialogText = "Error! UE4 only supports cubemaps with a maximum dimension of 2048x2048 per face";
							FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(DialogText));
							return;
						}
						else if (ImageWidth != ImageHeight * 12)
						{
							FString DialogText = "Error! StereoCubeMap's width is incorrect for its height. Width should be Height x 12.";
							FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(DialogText));
							return;
						}
						//Create the Package
						FString PackageName = "";
						FString TextureName = FPaths::GetCleanFilename(OutFileNames[0]);

						TextureName.RemoveFromEnd(FPaths::GetExtension(OutFileNames[0], true));
						TextureName.Append("_SC");

						FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
						FSaveAssetDialogConfig SaveAssetDialogConfig;
						SaveAssetDialogConfig.DialogTitleOverride = LOCTEXT("Save Stereo CubeMap", "Save Stereo Cubemap's As");
						SaveAssetDialogConfig.DefaultPath = TEXT("/Game");
						SaveAssetDialogConfig.DefaultAssetName = TextureName;
						SaveAssetDialogConfig.WindowSizeOverride = FVector2D(800.f, 600.f);
						SaveAssetDialogConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::AllowButWarn; 
						
						PackageName = ContentBrowserModule.Get().CreateModalSaveAssetDialog(SaveAssetDialogConfig);
						
						if (PackageName == "")
						{
							return;
						}

						PackageName.RemoveFromEnd(FPaths::GetExtension(PackageName, true));
						FString FinalLeftPackageName = PackageName;
						FinalLeftPackageName.Append("_LEFT"); 

						UPackage* Package = CreatePackage(nullptr, *FinalLeftPackageName);

						//Create the TextureCube
						UTextureCube* LeftCubeTexture = NewObject<UTextureCube>(Package, FName(*FPaths::GetBaseFilename(FinalLeftPackageName)), RF_Standalone | RF_Public);
						
						//No Mip Gen
						LeftCubeTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
						//no alpha saved
						LeftCubeTexture->CompressionNoAlpha = true;
						//Init
						LeftCubeTexture->Source.Init(ImageHeight, ImageHeight, 6, 1, ETextureSourceFormat::TSF_BGRA8);

						uint32 MipSize = CalculateImageBytes(ImageHeight, ImageHeight, 0/*should be 0*/, EPixelFormat::PF_B8G8R8A8);
						//Write to Mip
						uint8* MipData = LeftCubeTexture->Source.LockMip(0);
						//un mirrored order
						//static const uint8 faceorder[12] = { 3, 2, 4, 5, 0, 1, 3, 2, 4, 5, 0, 1 };
						static const uint8 faceorder[12] = { 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1 };
						
						FScopedSlowTask progress(18.f, LOCTEXT("Importing CubeMap...", "Importing CubeMap..."));
						progress.MakeDialog();
						progress.EnterProgressFrame(1.f, LOCTEXT("Generating Left CubeMap...", "Generating Left CubeMap..."));
						for (uint8 faceindex = 0; faceindex < 6; faceindex++)
						{
							FString notifystring = "Extracting cube face ";
							notifystring.AppendInt(faceindex+1);
							notifystring += "...";
							progress.EnterProgressFrame(1.f,FText::FromString(notifystring));
							GetCubeFaceData(RawData, faceindex, MipSize, ImageHeight, &FaceData);
							FMemory::Memcpy(MipData + (MipSize * faceorder[faceindex]), FaceData.GetData(), MipSize);
						}
						
						LeftCubeTexture->Source.UnlockMip(0);
						// Notify the asset registry
						progress.EnterProgressFrame(1.f, LOCTEXT("Creating LEFT asset...", "Creating LEFT asset..."));
						FAssetRegistryModule::AssetCreated(LeftCubeTexture);
						LeftCubeTexture->Modify();
						LeftCubeTexture->PostEditChange();
						progress.EnterProgressFrame(1.f, LOCTEXT("Left CubeMap Generated!", "Left CubeMap Generated!"));

						//////////////////////////////////////////////////////////////////////right//////////////////////////
						FaceData.Empty();
						//Create the Package
						FString FinalRightPackageName = PackageName;
						FinalRightPackageName.Append("_RIGHT");
						UPackage* Package2 = CreatePackage(nullptr, *PackageName);
						
						//Create the TextureCube
						UTextureCube* RightCubeTexture = NewObject<UTextureCube>(Package2, FName(*FPaths::GetBaseFilename(FinalRightPackageName)), RF_Standalone | RF_Public);
						//No Mip Gen
						RightCubeTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
						//no alpha saved
						RightCubeTexture->CompressionNoAlpha = true;
						//Init
						RightCubeTexture->Source.Init(ImageHeight, ImageHeight, 6, 1, ETextureSourceFormat::TSF_BGRA8);

						//Write to Mip
						MipData = RightCubeTexture->Source.LockMip(0);

						progress.EnterProgressFrame(1.f, LOCTEXT("Generating Right CubeMap...", "Generating Right CubeMap..."));
						for (uint8 faceindex = 6; faceindex < 12; faceindex++)
						{
							FString notifystring = "Extracting cube face ";
							notifystring.AppendInt(faceindex + 1);
							notifystring += "...";
							progress.EnterProgressFrame(1.f, FText::FromString(notifystring));
							GetCubeFaceData(RawData, faceindex, MipSize, ImageHeight, &FaceData);
							FMemory::Memcpy(MipData + (MipSize * faceorder[faceindex]), FaceData.GetData(), MipSize);
						}
						RightCubeTexture->Source.UnlockMip(0);
						// Notify the asset registry
						progress.EnterProgressFrame(1.f, LOCTEXT("Creating RIGHT asset...", "Creating RIGHT asset..."));
						FAssetRegistryModule::AssetCreated(RightCubeTexture);
						RightCubeTexture->PostEditChange();
						RightCubeTexture->MarkPackageDirty();
						progress.EnterProgressFrame(1.f, LOCTEXT("Right CubeMap Generated!", "Right CubeMap Generated!"));
						
						wrappersuccess = true;
					}

					break;
				}
			}
			if (!wrappersuccess)
			{
				FString DialogText = "ImageWrapper failed to decode file. Please try converting to PNG.";
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(DialogText));
			}

		}
		else
		{
			FString DialogText = "Error Loading File!" + OutFileNames[0];
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(DialogText));
		}

	}
}

bool FStereoCubeMapImporterModule::GetCubeFaceData(const TArray<uint8>* RawData, uint8 FaceIndex, uint32 MipSize, uint32 SizeX, TArray<uint8>* OutData)
{
	if (FaceIndex > 11 || MipSize == 0 || SizeX == 0) return false;
	uint32 ImageWidth = SizeX * 12;
	OutData->Empty(); //ensure data is empty
	OutData->SetNum(MipSize, true);
	//get a single face from the pano image
	for (uint32 i = 0; i < SizeX; i++)
	{
		uint32 panoLineOffset = (((ImageWidth * 4)) * i) + ((SizeX * 4) * FaceIndex);
		uint32 facelineoffset = ((SizeX * 4)) * i;
		FMemory::Memcpy(OutData->GetData() + facelineoffset, RawData->GetData() + panoLineOffset, SizeX * 4);
	}
	//rotate top bottom and front faces 90 anticlockwise 
	if (FaceIndex == 3 || FaceIndex == 9 || FaceIndex == 4 || FaceIndex == 10 || FaceIndex == 2 || FaceIndex == 8)
	{
		//RotateImageDataAntiClockwise90(OutData, SizeX);
		RotateImageData180(OutData, SizeX);
		RotateImageDataClockwise90(OutData, SizeX);
	}
	//rotate back faces 90 clockwises
	else if (FaceIndex == 5 || FaceIndex == 11 )
	{
		RotateImageDataClockwise90(OutData, SizeX);
	}
	// rotate right faces 180 degrees
	else if (FaceIndex == 0 || FaceIndex == 6)
	{
		RotateImageData180(OutData, SizeX);
	}
	
	return true;
}

void FStereoCubeMapImporterModule::RotateImageDataAntiClockwise90(TArray<uint8>* Data, uint32 SizeX)
{
	//todo: this is borked!!!!!!!!!!!!!!!!!! check later
	TArray<uint8> OriginalData = *Data;
	uint32 finalrowcount = 0;
	uint32 finalcolumbcount = 0;
	for (uint32 row = 0; row < SizeX; row++)
	{
		for (uint32 column = 1; column != SizeX + 1; column++)
		{
			uint32 destoffset = 4 * (column - 1) + (SizeX * 4 * row);
			uint32 srcoffset = (SizeX * 4 * column) - (row * 4);
			FMemory::Memcpy(Data->GetData() + destoffset, OriginalData.GetData() + srcoffset, 4);

			finalcolumbcount = column;
		}
		finalrowcount = row;
	}
}

void FStereoCubeMapImporterModule::RotateImageDataClockwise90(TArray<uint8>* Data, uint32 SizeX)
{
	TArray<uint8> OriginalData = *Data;
	int32 datasize = Data->Num();

	for (uint32 row = 0; row < SizeX; row++)
	{
		for (uint32 column = 1; column != SizeX + 1; column++)
		{
			uint32 destoffset = 4 * (column - 1) + (SizeX * 4 * row);
			uint32 srcoffset = datasize - (SizeX * 4 * column) + (row * 4);
			FMemory::Memcpy(Data->GetData() + destoffset, OriginalData.GetData() + srcoffset, 4);
		}
	}
}

void FStereoCubeMapImporterModule::RotateImageData180(TArray<uint8>* Data, uint32 SizeX)
{
	TArray<uint8> OriginalData = *Data;
	int32 datasize = Data->Num();

	for (uint32 i = 0; i < (SizeX*SizeX); i++)
	{
		uint32 destoffset = 4 * i;
		uint32 srcoffset = datasize - (4 * (i + 1));
		FMemory::Memcpy(Data->GetData() + destoffset, OriginalData.GetData() + srcoffset, 4);
	}
}

void FStereoCubeMapImporterModule::MirrorImageData(TArray<uint8>* Data, uint32 SizeX)
{
	TArray<uint8> OriginalData = *Data;
	uint32 RowLength = SizeX * 4;
	uint32 HalfRow = (uint32)ceilf(SizeX * 0.5);

	for (uint32 row = 0; row < SizeX; row++)
	{
		uint32 RowOffset = row * RowLength;
		for (uint32 column = 0; column < HalfRow; column++)
		{
			uint32 srcoffset = RowOffset + (column * 4);
			uint32 destoffset = RowOffset + RowLength - ((column + 1) * 4);
			FMemory::Memcpy(Data->GetData() + destoffset, OriginalData.GetData() + srcoffset, 4);
			FMemory::Memcpy(Data->GetData() + srcoffset, OriginalData.GetData() + destoffset, 4);
		}
	}
}

void FStereoCubeMapImporterModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FStereoCubeMapImporterCommands::Get().PluginAction);
}

void FStereoCubeMapImporterModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FStereoCubeMapImporterCommands::Get().PluginAction);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FStereoCubeMapImporterModule, StereoCubeMapImporter)

#endif