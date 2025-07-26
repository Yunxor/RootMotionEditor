// Copyright Epic Games, Inc. All Rights Reserved.

#include "RootMotionEditorModule.h"

#include "EditorModeRegistry.h"
#include "RMEStyle.h"
#include "RMECommands.h"
#include "RMEEdMode.h"
#include "SRootMotionEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

DEFINE_LOG_CATEGORY(LogRootMotionEditor);

FString FRootMotionEditorModule::RootMotionEditorLayoutIni;

#define LOCTEXT_NAMESPACE "FRootMotionEditorModule"

void FRootMotionEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	RootMotionEditorLayoutIni = GConfig->GetConfigFilename(TEXT("RootMotionEditorLayout"));
	
	FRMEStyle::Initialize();
	FRMEStyle::ReloadTextures();

	FRMECommands::Register();

	if (GIsEditor && !IsRunningCommandlet())
	{
		// Register Ed Modes used by PoseSearchDatabase and PoseSearchInteractionAsset
		FEditorModeRegistry::Get().RegisterMode<RootMotionEditor::FRMEEdMode>(RootMotionEditor::FRMEEdMode::EdModeId, LOCTEXT("RootMotionEditorEdModeName", "RootMotionEditor"));
	}

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FRMECommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FRootMotionEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FRootMotionEditorModule::RegisterMenus));

	// Any attempt to use GEditor right now will fail as it hasn't been initialized yet. Waiting for post engine init resolves that.
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FRootMotionEditorModule::OnPostEngineInit);
	FCoreDelegates::OnEnginePreExit.AddRaw(this, &FRootMotionEditorModule::OnPreExit);
}

void FRootMotionEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
	FEditorModeRegistry::Get().UnregisterMode(RootMotionEditor::FRMEEdMode::EdModeId);
	
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FRMEStyle::Shutdown();

	FRMECommands::Unregister();
}

FRootMotionEditorModule& FRootMotionEditorModule::Get()
{
	return FModuleManager::LoadModuleChecked<FRootMotionEditorModule>("RootMotionEditor");
}

TSharedRef<FRMEContext> FRootMotionEditorModule::GetContext()
{
	return Context.ToSharedRef();
}


void FRootMotionEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(SRootMotionEditor::WindowName);
}

void FRootMotionEditorModule::OnPostEngineInit()
{
	Context = MakeShared<FRMEContext>();
	Context->Initialize();
	
	SRootMotionEditor::RegisterTabSpawner();
}

void FRootMotionEditorModule::OnPreExit()
{
	SRootMotionEditor::UnregisterTabSpawner();
}

void FRootMotionEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FRMECommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FRMECommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRootMotionEditorModule, RootMotionEditor)