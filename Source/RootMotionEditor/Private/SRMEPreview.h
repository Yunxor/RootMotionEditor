// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SRMEViewport.h"

/**
 * 
 */
class SRMEPreview : public SCompoundWidget
{
public:
	DECLARE_DELEGATE_TwoParams(FOnScrubPositionChanged, double, bool)
	DECLARE_DELEGATE(FOnButtonClickedEvent)

	SLATE_BEGIN_ARGS(SRMEPreview) {}
	SLATE_ATTRIBUTE(FLinearColor, SliderColor);
	SLATE_ATTRIBUTE(double, SliderScrubTime);
	SLATE_ATTRIBUTE(TRange<double>, SliderViewRange);
	SLATE_EVENT(FOnScrubPositionChanged, OnSliderScrubPositionChanged);
	SLATE_EVENT(FOnButtonClickedEvent, OnBackwardEnd);
	SLATE_EVENT(FOnButtonClickedEvent, OnBackwardStep);
	SLATE_EVENT(FOnButtonClickedEvent, OnBackward);
	SLATE_EVENT(FOnButtonClickedEvent, OnPause);
	SLATE_EVENT(FOnButtonClickedEvent, OnForward);
	SLATE_EVENT(FOnButtonClickedEvent, OnForwardStep);
	SLATE_EVENT(FOnButtonClickedEvent, OnForwardEnd);
	SLATE_END_ARGS();


public:
	const static FName TabName;

public:
	static void RegisterTabSpawner(const TSharedPtr<FTabManager>& TabManager);
	

	void Construct(const FArguments& InArgs, const FRMEPreviewRequiredArgs& InRequiredArgs);

protected:
	TAttribute<FLinearColor> SliderColor;
	TAttribute<double> SliderScrubTime;
	TAttribute<TRange<double>> SliderViewRange = TRange<double>(0.0, 1.0);
	FOnScrubPositionChanged OnSliderScrubPositionChanged;
	FOnButtonClickedEvent OnBackwardEnd;
	FOnButtonClickedEvent OnBackwardStep;
	FOnButtonClickedEvent OnBackward;
	FOnButtonClickedEvent OnPause;
	FOnButtonClickedEvent OnForward;
	FOnButtonClickedEvent OnForwardStep;
	FOnButtonClickedEvent OnForwardEnd;
};