// Copyright Epic Games, Inc. All Rights Reserved.

#include "RMECommands.h"

#define LOCTEXT_NAMESPACE "FRootMotionEditorModule"

void FRMECommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Root Motion Editor", "Bring up RootMotionEditor window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND( RootMotionNone, "None", "Hide root motion", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND( RootMotionFromAnimAsset, "Enable Root Motion From Asset", "Enable root motion, this data from selected animation Asset", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND( RootMotionFromCurveEditor, "Enable Root Motion From Editor", "Enable root motion, this data from curve editor", EUserInterfaceActionType::RadioButton, FInputChord());

	UI_COMMAND(DoNotVisualizeRootMotion, "None", "Do not show root motion", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(VisualizeRootMotionTrajectory, "Visualize Trajectory", "Show root motion trajectory", EUserInterfaceActionType::RadioButton, FInputChord());
	UI_COMMAND(VisualizeRootMotionTrajectoryAndOrientation, "Visualize Trajectory and Orientation", "Show root motion trajectory and orientation", EUserInterfaceActionType::RadioButton, FInputChord());
}

#undef LOCTEXT_NAMESPACE
