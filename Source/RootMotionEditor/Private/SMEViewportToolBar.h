// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SCommonEditorViewportToolbarBase.h"


class SRMEViewport;
/**
 * 
 */
class SMEViewportToolBar : public SCommonEditorViewportToolbarBase
{
public:
	SLATE_BEGIN_ARGS(SMEViewportToolBar)
	{}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<SRMEViewport> InViewport);

	// ~SCommonEditorViewportToolbarBase interface
	virtual TSharedRef<SWidget> GenerateShowMenu() const override;
	// ~End of SCommonEditorViewportToolbarBase interface
};
