// Fill out your copyright notice in the Description page of Project Settings.


#include "SRMECurveSelector.h"
#include "PropertyCustomizationHelpers.h"
#include "Curves/CurveVector.h"
#include "ThumbnailRendering/ThumbnailManager.h"


#define LOCTEXT_NAMESPACE "RootMotionEditedCurveAssets"

SRMECurveSelector::SRMECurveSelector()
{
}

SRMECurveSelector::~SRMECurveSelector()
{
}

void SRMECurveSelector::Construct(const FArguments& InArgs)
{
	MotionCurve = InArgs._MotionCurve;
	RotationCurve = InArgs._RotationCurve;
	ScaleCurve = InArgs._ScaleCurve;
	
	TSharedRef<SVerticalBox> VerticalBox = SNew(SVerticalBox);

	TArray<const UClass*> AllowedClasses;
	AllowedClasses.Add(UCurveVector::StaticClass());
	
	auto AddCurveEntryBox = [this, &VerticalBox, &AllowedClasses](const FText& Text, TAttribute<FSoftObjectPath>& CurvePath)
	{
		VerticalBox->AddSlot()
		.Padding(2.f)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.FillWidth(0.4f)
			[
				SNew(STextBlock)
				.Text(Text)
			]
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.FillWidth(0.6f)
			[
				SNew(SObjectPropertyEntryBox)
				.AllowedClass(UCurveVector::StaticClass())
				.NewAssetFactories(PropertyCustomizationHelpers::GetNewAssetFactoriesForClasses(AllowedClasses))
				.AllowCreate(true)
				.ThumbnailPool(UThumbnailManager::Get().GetSharedThumbnailPool())
				.DisplayThumbnail(true)
				.ObjectPath_Lambda([&CurvePath]()
				{
					return CurvePath.Get().ToString();
				})
				.OnObjectChanged_Lambda([&CurvePath](const FAssetData& AssetData)
				{
					CurvePath.Set(AssetData.GetSoftObjectPath());
				})
			]
		];
		VerticalBox->AddSlot()
		.AutoHeight()
		.Padding(0, 5, 0, 5)
		[
			SNew(SSeparator)
			.Orientation(Orient_Horizontal)
		];
	};

	AddCurveEntryBox(FText::FromString("Motion Curve"), MotionCurve);
	AddCurveEntryBox(FText::FromString("Rotation Curve"), RotationCurve);
	AddCurveEntryBox(FText::FromString("Scale Curve"), ScaleCurve);

	
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.FillHeight(0.2f)
			.Padding(5.f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString("Edited Curve Assets"))
				.Font(FAppStyle::GetFontStyle("HeadingExtraSmall"))
				.Justification(ETextJustify::Center)
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				VerticalBox
			]
		]
	];
}

bool SRMECurveSelector::HasAnyCurveAsset() const
{
	return !MotionCurve.Get().IsNull() || !RotationCurve.Get().IsNull() || !ScaleCurve.Get().IsNull();
}

UCurveVector* SRMECurveSelector::GetCurveAsset(ERMECurveType CurveType)
{
	TAttribute<FSoftObjectPath>* Curve = nullptr;
	switch (CurveType) {
	case ERMECurveType::Motion:
		Curve = &MotionCurve;
		break;
	case ERMECurveType::Rotation:
		Curve = &RotationCurve;
		break;
	case ERMECurveType::Scale:
		Curve = &ScaleCurve;
		break;
	}

	const FSoftObjectPath& CurvePath = Curve->Get();
	return Cast<UCurveVector>(CurvePath.TryLoad());
}

#undef LOCTEXT_NAMESPACE
