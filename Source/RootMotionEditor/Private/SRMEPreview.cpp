// Fill out your copyright notice in the Description page of Project Settings.


#include "SRMEPreview.h"

#include "RootMotionEditorModule.h"
#include "SRMEViewport.h"
#include "SSimpleTimeSlider.h"

#define LOCTEXT_NAMESPACE "RootMotionEditedCurveAssets"

const FName SRMEPreview::TabName = FName(TEXT("RootMotionEditorPreviewTab"));

void SRMEPreview::RegisterTabSpawner(const TSharedPtr<FTabManager>& TabManager, TSharedRef<FRMEContext> Context)
{
	FRootMotionEditorPreviewRequiredArgs RequiredArgs = Context->MakePreviewRequiredArgs();
	
	TabManager->RegisterTabSpawner(
			TabName,
			FOnSpawnTab::CreateLambda(
				[=](const FSpawnTabArgs&)
				{
					return SNew(SDockTab)
						.TabRole(ETabRole::PanelTab)
						.Label(LOCTEXT("PreviewTitle", "Root Motion Editor Preview"))
						[
							SNew(SRMEPreview, RequiredArgs)
							.SliderColor_Lambda([&Context]()
							{
								return Context->IsEditorSelection() ? FLinearColor::Red.CopyWithNewOpacity(0.5f) : FLinearColor::Blue.CopyWithNewOpacity(0.5f);
							})
							.SliderScrubTime_Lambda([&Context]()
							{
								return Context->GetViewModelPlayTime();
							})
							.SliderViewRange_Lambda([&Context]()
							{
								return Context->GetViewModelPlayTimeRange();
							})
							.OnSliderScrubPositionChanged_Lambda([&Context](float NewScrubPosition, bool bScrubbing)
							{
								Context->SetViewModelPlayTime(NewScrubPosition, !bScrubbing);
							})
							.OnBackwardEnd_Raw(&Context.Get(), &FRMEContext::PreviewBackwardEnd)
							.OnBackwardStep_Raw(&Context.Get(), &FRMEContext::PreviewBackwardStep)
							.OnBackward_Raw(&Context.Get(), &FRMEContext::PreviewBackward)
							.OnPause_Raw(&Context.Get(), &FRMEContext::PreviewPause)
							.OnForward_Raw(&Context.Get(), &FRMEContext::PreviewForward)
							.OnForwardStep_Raw(&Context.Get(), &FRMEContext::PreviewForwardStep)
							.OnForwardEnd_Raw(&Context.Get(), &FRMEContext::PreviewForwardEnd)
						];
				}
			)
		)
		.SetDisplayName(LOCTEXT("PreviewTabTitle", "Root Motion Editor Preview"))
		.SetTooltipText(LOCTEXT("PreviewTooltipText", "Open the root motion edited preview tab."));
}

void SRMEPreview::Construct(const FArguments& InArgs, const FRootMotionEditorPreviewRequiredArgs& InRequiredArgs)
{
	SliderColor = InArgs._SliderColor;
	SliderScrubTime = InArgs._SliderScrubTime;
	SliderViewRange = InArgs._SliderViewRange;
	OnSliderScrubPositionChanged = InArgs._OnSliderScrubPositionChanged;

	OnBackwardEnd = InArgs._OnBackwardEnd;
	OnBackwardStep = InArgs._OnBackwardStep;
	OnBackward = InArgs._OnBackward;
	OnPause = InArgs._OnPause;
	OnForward = InArgs._OnForward;
	OnForwardStep = InArgs._OnForwardStep;
	OnForwardEnd = InArgs._OnForwardEnd;

	FSlimHorizontalToolBarBuilder ToolBarBuilder(
		TSharedPtr<const FUICommandList>(), 
		FMultiBoxCustomization::None, 
		nullptr, true);

	auto AddToolBarButton = [&ToolBarBuilder](FName ButtonImageName, FOnButtonClickedEvent& OnClicked)
		{
			ToolBarBuilder.AddToolBarWidget(
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "Animation.PlayControlsButton")
				.OnClicked_Lambda([&OnClicked]()
					{
						if (OnClicked.IsBound())
						{
							OnClicked.Execute();
							return FReply::Handled();
						}
						return FReply::Unhandled();
					})
				[
					SNew(SImage)
						.ColorAndOpacity(FSlateColor::UseSubduedForeground())
						.Image(FAppStyle::Get().GetBrush(ButtonImageName))
				]);
		};

	//ToolBarBuilder.SetStyle(&FAppStyle::Get(), "PaletteToolBar");
	ToolBarBuilder.BeginSection("Preview");
	{
		AddToolBarButton("Animation.Backward_End", OnBackwardEnd);
		AddToolBarButton("Animation.Backward_Step", OnBackwardStep);
		AddToolBarButton("Animation.Backward", OnBackward);
		AddToolBarButton("Animation.Pause", OnPause);
		AddToolBarButton("Animation.Forward", OnForward);
		AddToolBarButton("Animation.Forward_Step", OnForwardStep);
		AddToolBarButton("Animation.Forward_End", OnForwardEnd);
	}

	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SRMEViewport, InRequiredArgs)
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				ToolBarBuilder.MakeWidget()
			]
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SSimpleTimeSlider)
				.ClampRangeHighlightSize(0.15f)
				.ClampRangeHighlightColor_Lambda([this]()
					{
						return SliderColor.Get();
					})
				.ScrubPosition_Lambda([this]()
					{
						return SliderScrubTime.Get();
					})
				.ViewRange_Lambda([this]()
					{
						return SliderViewRange.Get();
					})
				.ClampRange_Lambda([this]()
					{
						return SliderViewRange.Get();
					})
				.OnScrubPositionChanged_Lambda([this](double NewScrubTime, bool bIsScrubbing)
					{
						if (bIsScrubbing)
						{
							OnSliderScrubPositionChanged.ExecuteIfBound(NewScrubTime, bIsScrubbing);
						}
					})
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE