// Fill out your copyright notice in the Description page of Project Settings.


#include "RMETypes.h"
#include "Curves/CurveVector.h"

DEFINE_LOG_CATEGORY(LogRootMotionEditor);


FLinearColor URMECurveContainer::GetCurveAxisColor(const int32& Index)
{
	static const FLinearColor Colors[3] =
	{
		FLinearColor::Red,
		FLinearColor::Green,
		FLinearColor::Blue,
	};

	return Colors[Index % 3];
}

FVectorCurve URMECurveContainer::ConvertToFCurve(class UCurveVector* InCurveAsset)
{
	FVectorCurve Result;
	if (InCurveAsset != nullptr)
	{
		Result.FloatCurves[0] = InCurveAsset->FloatCurves[0];
		Result.FloatCurves[1] = InCurveAsset->FloatCurves[1];
		Result.FloatCurves[2] = InCurveAsset->FloatCurves[2];
	}
	return Result;
}

void URMECurveContainer::BeginDestroy()
{
	if (IsNew() && CurveData)
    {
    	DeleteCurveData();
    }
    MakeDestroy();
	
	UObject::BeginDestroy();
}

URMECurveContainer* URMECurveContainer::Create(FTransformCurve* SourceData, bool bAddToRoot)
{
	URMECurveContainer* NewContainer= NewObject<URMECurveContainer>();
	if (SourceData != nullptr)
	{
		NewContainer->CurveData = SourceData;
	}
	else
	{
		NewContainer->GetOrCreateCurveData();
	}

	if (bAddToRoot)
	{
		NewContainer->AddToRoot();
		NewContainer->bIsAddToRoot = true;
	}

	return NewContainer;
}

void URMECurveContainer::ClearAllKeys()
{
	if (CurveData != nullptr)
	{
		auto ResetKeys = [](FVectorCurve& InCurve)
		{
			for (int Index = 0; Index < 3; ++Index)
			{
				FRichCurve& Curve = InCurve.FloatCurves[Index];
				Curve.Reset();
			}
		};

		ResetKeys(CurveData->TranslationCurve);
		ResetKeys(CurveData->RotationCurve);
		ResetKeys(CurveData->ScaleCurve);
	}
}

void URMECurveContainer::DeleteCurveData()
{
	if (CurveData != nullptr)
	{
		delete CurveData;
		CurveData = nullptr;
		bIsNew = false;
	}
}

void URMECurveContainer::MakeDestroy()
{
	if (bIsAddToRoot)
	{
		this->RemoveFromRoot();
		this->ConditionalBeginDestroy();
		bIsAddToRoot = false;
	}
}

FTransformCurve* URMECurveContainer::GetOrCreateCurveData()
{
	if (CurveData == nullptr)
	{
		CurveData = new FTransformCurve;
		bIsNew = true;
	}
	
	return CurveData;
}

void URMECurveContainer::PushCurveData(class UCurveVector* Motion, class UCurveVector* Rotation, class UCurveVector* Scale)
{
	if (CurveData == nullptr)
	{
		CurveData = new FTransformCurve;
		bIsNew = true;
	}
	CurveData->TranslationCurve = ConvertToFCurve(Motion);
	CurveData->RotationCurve = ConvertToFCurve(Rotation);
	CurveData->ScaleCurve = ConvertToFCurve(Scale);
}

void URMECurveContainer::OverrideCurveData(FTransformCurve& NewCurveData)
{
	if (CurveData != nullptr && IsNew())
	{
		DeleteCurveData();
	}
	CurveData = &NewCurveData;
}

void URMECurveContainer::CopyCurveData(const FTransformCurve& NewCurveData)
{
	CurveData->CopyCurve(NewCurveData);

}

FName URMECurveContainer::GetFullCurveName(FString InName, int32 Index)
{
	static const FString AxisSuffix[3] =
	{
		FString(".X"),
		FString(".Y"),
		FString(".Z"),
	};

	InName += AxisSuffix[Index % 3];

	return FName(InName);
}


#if WITH_EDITOR
bool URMECurveEditorConfig::CanEditChange(const FEditPropertyChain& PropertyChain) const
{
	const bool ParentVal = UObject::CanEditChange(PropertyChain);

	const FProperty* Property = PropertyChain.GetActiveNode() ? PropertyChain.GetActiveNode()->GetValue() : nullptr;
	if (Property)
	{
		const FName PropertyName = Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(URMECurveEditorConfig, EvaluationOptions)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(URMECurveEditorConfig, Space)
			)
		{
			return ParentVal && !CustomLoadBoneName.IsNone();
		}
	}

	return ParentVal;
}

bool URMEAssetCollection::HasAnyCurveAsset() const
{
	return MotionCurve || RotationCurve || ScaleCurve;
}

bool URMEAssetCollection::HasRepeatedCurve() const
{
	TSet<UCurveVector*> Curves;
	bool bHasRepeated = false;
	auto CheckFunction = [&Curves, &bHasRepeated](UCurveVector* InCurve)
	{
		if (InCurve != nullptr)
		{
			if (Curves.Contains(InCurve))
			{
				bHasRepeated = true;
			}
			else
			{
				Curves.Add(InCurve);
			}
		}
	};

	CheckFunction(MotionCurve);
	CheckFunction(RotationCurve);
	CheckFunction(ScaleCurve);

	return bHasRepeated;
}
#endif
