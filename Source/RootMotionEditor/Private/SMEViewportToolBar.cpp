// Fill out your copyright notice in the Description page of Project Settings.


#include "SMEViewportToolBar.h"
#include "SRMEViewport.h"
#include "PreviewProfileController.h"
#include "RMECommands.h"
#include "Widgets/Text/SRichTextBlock.h"

#define LOCTEXT_NAMESPACE "RootMotionEditedViewportToolBar"

void SMEViewportToolBar::Construct(const FArguments& InArgs, TSharedPtr<SRMEViewport> InViewport)
{
	SCommonEditorViewportToolbarBase::Construct(
			SCommonEditorViewportToolbarBase::FArguments()
			.AddRealtimeButton(false)
			.PreviewProfileController(MakeShared<FPreviewProfileController>()),
			InViewport);

	TSharedRef<SWidget> ParentWidget = ChildSlot.GetWidget();

	TSharedRef<SVerticalBox> TotalWidget = SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			ParentWidget
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(4.0f, 3.0f, 0.0f, 0.0f))
		[
			// Display text (e.g., item being previewed)
			SNew(SRichTextBlock)
			.Visibility(EVisibility::SelfHitTestInvisible)
			.DecoratorStyleSet(&FAppStyle::Get())
			.Text(InViewport.Get(), &SRMEViewport::GetDisplayString)
			.TextStyle(&FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("AnimViewport.MessageText"))
		];

	ChildSlot.AttachWidget(TotalWidget);
}

TSharedRef<SWidget> SMEViewportToolBar::GenerateShowMenu() const
{
	GetInfoProvider().OnFloatingButtonClicked();

	TSharedRef<SEditorViewport> ViewportRef = GetInfoProvider().GetViewportWidget();

	const bool bInShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder ShowMenuBuilder(bInShouldCloseWindowAfterMenuSelection, ViewportRef->GetCommandList());
	{
		const FRMECommands& Commands = FRMECommands::Get();
		
		{
			ShowMenuBuilder.BeginSection("Debug", LOCTEXT("ShowMenu_DebugLabel", "Debug"));
			ShowMenuBuilder.AddMenuEntry(Commands.RootMotionNone);
			ShowMenuBuilder.AddMenuEntry(Commands.RootMotionFromAnimAsset);
			ShowMenuBuilder.AddMenuEntry(Commands.RootMotionFromCurveEditor);
			ShowMenuBuilder.EndSection();
		}
		
		// TODO: bind view trajectory.
		{
			ShowMenuBuilder.BeginSection("AnimViewportVisualization", LOCTEXT("CharacterMenu_VisualizationsLabel", "Visualizations"));
			ShowMenuBuilder.AddMenuEntry(Commands.DoNotVisualizeRootMotion);
			ShowMenuBuilder.AddMenuEntry(Commands.VisualizeRootMotionTrajectory);
			ShowMenuBuilder.AddMenuEntry(Commands.VisualizeRootMotionTrajectoryAndOrientation);
			ShowMenuBuilder.EndSection();
		}
	}

	return ShowMenuBuilder.MakeWidget();
}

#undef LOCTEXT_NAMESPACE