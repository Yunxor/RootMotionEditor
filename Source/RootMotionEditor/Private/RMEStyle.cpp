// Copyright Epic Games, Inc. All Rights Reserved.

#include "RMEStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FRMEStyle::StyleInstance = nullptr;

void FRMEStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FRMEStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FRMEStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("RootMotionEditorStyle"));
	return StyleSetName;
}

const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef< FSlateStyleSet > FRMEStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("RootMotionEditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("RootMotionEditor")->GetBaseDir() / TEXT("Resources"));

	Style->Set("RootMotionEditor.OpenPluginWindow", new IMAGE_BRUSH(TEXT("OpenButtonIcon"), Icon40x40));
	

	return Style;
}

void FRMEStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FRMEStyle::Get()
{
	return *StyleInstance;
}
