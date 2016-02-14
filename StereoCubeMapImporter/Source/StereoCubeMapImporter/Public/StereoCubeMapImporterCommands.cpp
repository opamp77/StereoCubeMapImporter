// Some copyright should be here...

#include "StereoCubeMapImporterPrivatePCH.h"
#include "StereoCubeMapImporterCommands.h"

#define LOCTEXT_NAMESPACE "FStereoCubeMapImporterModule"

void FStereoCubeMapImporterCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "StereoCubeMapImporter", "Execute StereoCubeMapImporter action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
