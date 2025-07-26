// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"


/**
 * 
 */
class SRootMotionEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRootMotionEditor) { }
		SLATE_ARGUMENT(TSharedPtr<class FTabManager>, TabManager)
	SLATE_END_ARGS()

	
public:
	static const FName WindowName;
	
public:
	static void RegisterTabSpawner();
	static void UnregisterTabSpawner();
	static TSharedRef<class SDockTab> SpawnRootMotionEditor(const class FSpawnTabArgs& Args);
	/**
	* Default constructor.
	*/
	SRootMotionEditor();
	virtual ~SRootMotionEditor();


	void Construct(const FArguments& InArgs);
	
	void FillWindowMenu(FMenuBuilder& MenuBuilder);
	
protected:
	TSharedRef<SWidget> MakeToolbar();
	
	void NotifyCurveChanged();
	

private:
	TSharedPtr<class SRMEAssetsSelector> ViewAssetPanel;
	
	TSharedPtr<class SRMECurveSelector> CurvesAssetPanel;
	
	TSharedPtr<class SRMEPreview> PreviewPanel;

	TSharedPtr<class FRMECurveEditor> CurveEditor;

	TSharedPtr<FTabManager> TabManager;
};
