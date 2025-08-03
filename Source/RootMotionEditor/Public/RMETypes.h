// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimPose.h"
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

UENUM(DisplayName = "Bone Extract Channel Type", meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ERMEBoneExtractChannelType : uint32
{
	None = 0 UMETA(Hidden),
	Translation = 1 << 0,
	Rotation	= 1	<< 1,
	Scale		= 1	<< 2,

	All = Translation | Rotation | Scale,
};

ENUM_CLASS_FLAGS(ERMEBoneExtractChannelType);
constexpr bool EnumHasAnyFlags(int32 Flags, ERMEBoneExtractChannelType Contains) { return (Flags & static_cast<int32>(Contains)) != 0; }


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
	void ClearAllKeys();
	void DeleteCurveData();
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Load")
	FName CustomLoadBoneName = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Load", meta = (Bitmask, BitmaskEnum = "/Script/RootMotionEditor.ERMEBoneExtractChannelType"))
	int32 ExtractChannels = int32(ERMEBoneExtractChannelType::Translation);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Load")
	int32 SampleRate = 30;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Load", meta = (ToolTip = "If it is true, it will make every frame of the curve is motion delta."))
	bool bIsAdditiveCurve = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Load")
	FAnimPoseEvaluationOptions EvaluationOptions = FAnimPoseEvaluationOptions();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Load")
	EAnimPoseSpaces Space = EAnimPoseSpaces::World;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FName CustomSaveBoneName = NAME_None;


#if WITH_EDITOR
	virtual bool CanEditChange(const FEditPropertyChain& PropertyChain) const override;
#endif
};

UCLASS()
class URMEAssetCollection : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	TObjectPtr<class UAnimSequence> AnimSequence = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Curve", meta = (AllowedClasses = "/Script/Engine.CurveBase", DisallowedClasses = "/Script/Engine.CurveLinearColor, /Script/Engine.CurveFloat"))
	TObjectPtr<class UCurveVector> MotionCurve = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Curve", meta = (AllowedClasses = "/Script/Engine.CurveBase", DisallowedClasses = "/Script/Engine.CurveLinearColor, /Script/Engine.CurveFloat"))
	TObjectPtr<class UCurveVector> RotationCurve = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Curve", meta = (AllowedClasses = "/Script/Engine.CurveBase", DisallowedClasses = "/Script/Engine.CurveLinearColor, /Script/Engine.CurveFloat"))
	TObjectPtr<class UCurveVector> ScaleCurve = nullptr;
	
	bool HasAnyCurveAsset() const;

	bool HasRepeatedCurve() const;
};
