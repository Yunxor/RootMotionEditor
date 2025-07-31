// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RMETypes.generated.h"

namespace ERootMotionViewMode
{
	enum Type
	{
		None = 0,
		AnimAsset,
		CurveEditor,
	};

	static FString GetDisplayName(int32 InMode)
	{
		FString DisplayName = TEXT("None");
		const ERootMotionViewMode::Type Type = static_cast<ERootMotionViewMode::Type>(InMode);
		switch (Type)
		{
		case None:
			DisplayName = TEXT("None");
			break;
		case AnimAsset:
			DisplayName = TEXT("AnimAsset");
			break;
		case CurveEditor:
			DisplayName = TEXT("CurveEditor");
			break;
		}
		return DisplayName;
	}
}

UCLASS()
class URMECurveContainer : public UObject
{
	GENERATED_BODY()
public:
	static FName GetFullCurveName(FString InName, int32 Index);
	static  FLinearColor GetCurveAxisColor(const int32& Index);

	static FVectorCurve ConvertToFCurve(class UCurveVector* InCurveAsset);

public:
	URMECurveContainer(){};

	URMECurveContainer(FTransformCurve& InCurveData)
		:CurveData(&InCurveData), bIsNew(true)
	{}

	virtual void BeginDestroy() override;

	static URMECurveContainer* Create(FTransformCurve* SourceData = nullptr, bool bAddToRoot = false);
	void ClearCurveData();
	void MakeDestroy();

	FTransformCurve* GetOrCreateCurveData();

	void PushCurveData(class UCurveVector* Motion, class UCurveVector* Rotation, class UCurveVector* Scale);
	void OverrideCurveData(FTransformCurve& NewCurveData);
	void CopyCurveData(const FTransformCurve& NewCurveData);

	bool IsNew() const { return bIsNew; }

	
	FTransformCurve* CurveData = nullptr;

private:
	bool bIsNew = false;
	bool bIsAddToRoot = false;
};

UCLASS(BlueprintType)
class URMECurveEditorConfig : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LoadBoneName = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SaveBoneName = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAdditiveCurve = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SampleRate = 30;
};

UCLASS()
class URMEAssetCollection : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	TObjectPtr<class UAnimSequence> AnimSequence = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Curve")
	TObjectPtr<class UCurveVector> MotionCurve = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Curve")
	TObjectPtr<class UCurveVector> RotationCurve = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Curve")
	TObjectPtr<class UCurveVector> ScaleCurve = nullptr;

	bool HasAnyCurveAsset() const
	{
		return MotionCurve || RotationCurve || ScaleCurve;
	}
};
