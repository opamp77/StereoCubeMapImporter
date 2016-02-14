// Some copyright should be here...

#pragma once

#include "SlateBasics.h"
#include "StereoCubeMapImporterStyle.h"

class FStereoCubeMapImporterCommands : public TCommands<FStereoCubeMapImporterCommands>
{
public:

	FStereoCubeMapImporterCommands()
		: TCommands<FStereoCubeMapImporterCommands>(TEXT("StereoCubeMapImporter"), NSLOCTEXT("Contexts", "StereoCubeMapImporter", "StereoCubeMapImporter Plugin"), NAME_None, FStereoCubeMapImporterStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
