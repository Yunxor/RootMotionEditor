// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "RMEContext.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

DECLARE_LOG_CATEGORY_EXTERN(LogRootMotionEditor, Log, All);

class FRootMotionEditorModule : public IModuleInterface
{
public:
	static FString RootMotionEditorLayoutIni;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Get the instance of this module. */
	ROOTMOTIONEDITOR_API static FRootMotionEditorModule& Get();

	TSharedRef<FRMEContext> GetContext();
	
	void PluginButtonClicked();

	void OnPostEngineInit();
	void OnPreExit();
	
private:
	void RegisterMenus();

private:
	TSharedPtr<class FUICommandList> PluginCommands;
	
	TSharedPtr<FRMEContext> Context;

};
