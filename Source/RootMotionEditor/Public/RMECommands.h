// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "RMEStyle.h"

class FRMECommands : public TCommands<FRMECommands>
{
public:

	FRMECommands()
		: TCommands<FRMECommands>(TEXT("RootMotionEditor"), NSLOCTEXT("Contexts", "RootMotionEditor", "RootMotionEditor Plugin"), NAME_None, FRMEStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
	TSharedPtr< FUICommandInfo > RootMotionNone;
	TSharedPtr< FUICommandInfo > RootMotionFromAnimAsset;
	TSharedPtr< FUICommandInfo > RootMotionFromCurveEditor;

	/** Visualize root motion mode */
	TSharedPtr< FUICommandInfo > DoNotVisualizeRootMotion;
	TSharedPtr< FUICommandInfo > VisualizeRootMotionTrajectory;
	TSharedPtr< FUICommandInfo > VisualizeRootMotionTrajectoryAndOrientation;
};