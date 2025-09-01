// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FRootMotionEditorModule : public IModuleInterface
{
public:
	static FString RootMotionEditorLayoutIni;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Get the instance of this module. */
	ROOTMOTIONEDITOR_API static FRootMotionEditorModule& Get();
	
	void PluginButtonClicked();

	void OnPostEngineInit();
	void OnPreExit();
	
private:
	void RegisterMenus();

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};
