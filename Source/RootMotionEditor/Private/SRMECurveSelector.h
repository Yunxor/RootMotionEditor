// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveVector.h"
#include "Widgets/SCompoundWidget.h"

UENUM()
enum class ERMECurveType : uint8
{
	Motion = 0,
	Rotation,
	Scale,
};

/**
 * 
 */
class SRMECurveSelector : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRMECurveSelector) { }
		SLATE_ATTRIBUTE(FSoftObjectPath, MotionCurve)
		SLATE_ATTRIBUTE(FSoftObjectPath, RotationCurve)
		SLATE_ATTRIBUTE(FSoftObjectPath, ScaleCurve)
	SLATE_END_ARGS()

public:
	SRMECurveSelector();
	virtual ~SRMECurveSelector();

	void Construct(const FArguments& InArgs);
	bool HasAnyCurveAsset() const;
	UCurveVector* GetCurveAsset(ERMECurveType CurveType);

private:
	TAttribute<FSoftObjectPath> MotionCurve;
	TAttribute<FSoftObjectPath> RotationCurve;
	TAttribute<FSoftObjectPath> ScaleCurve;
};
