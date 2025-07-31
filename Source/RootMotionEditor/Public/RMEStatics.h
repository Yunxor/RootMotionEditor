#pragma once
#include "AnimPose.h"
#include "DynamicMeshBuilder.h"


namespace RootMotionEditorStatics
{
	template<class T>
	static TSharedPtr<T> GetTabWidget(FTabManager* TabManager, const FName& TabName)
	{
		TSharedPtr<T> Result;
		if (TabManager)
		{
			if (TSharedPtr<SDockTab> DebugTab = TabManager->FindExistingLiveTab(TabName))
			{
				Result = StaticCastSharedRef<T>(DebugTab->GetContent());
			}
		}
		return Result;
	}

	static bool IsValidBoneName(const UAnimSequence* AnimSequence, const FName& BoneName)
	{
		if (AnimSequence != nullptr)
		{
			if (const USkeleton* Skeleton = AnimSequence->GetSkeleton())
			{
				const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
				if (RefSkeleton.GetNum() > 0)
				{
					return RefSkeleton.FindBoneIndex(BoneName) != INDEX_NONE;
				}
			}
		}

		return false;
	}

	static FName GetRootBoneName(const UAnimSequence* AnimSequence)
	{
		if (AnimSequence != nullptr)
		{
			if (const USkeleton* Skeleton = AnimSequence->GetSkeleton())
			{
				const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
				if (RefSkeleton.GetNum() > 0)
				{
					return RefSkeleton.GetBoneName(0);
				}
			}
		}

		return NAME_None;
	}

	static FTransformCurve BakeRootBoneToCurves(UAnimSequence* AnimSequence, bool bIsAdditiveCurve = false, int32 SampleRate = 30, FName CustomExtractBone = NAME_None)
	{
		if (!AnimSequence)
		{
			UE_LOG(LogAnimation, Warning, TEXT("Invalid AnimSequence"));
			return FTransformCurve();
		}

		const bool bIsCustomExtractBone = CustomExtractBone.IsValid();

		if (!bIsCustomExtractBone && !AnimSequence->HasRootMotion())
		{
			UE_LOG(LogAnimation, Warning, TEXT("AnimSequence haven't root motion."));
			return FTransformCurve();
		}

		const float SampleInterval = 1.f / static_cast<float>(SampleRate);
		const float AnimLength = AnimSequence->GetPlayLength();

		FTransformCurve Result;
		float Time = 0.0f;
		FTransform LastRootMotion = FTransform::Identity;
		while (Time < AnimLength)
		{
			FTransform RootMotionDelta;
			if (!bIsCustomExtractBone)
			{
				RootMotionDelta = AnimSequence->ExtractRootMotion(Time, SampleInterval, false);
			}
			else
			{
				FAnimPoseEvaluationOptions EvaluationOptions = FAnimPoseEvaluationOptions();
				FAnimPose AnimPose;
				UAnimPoseExtensions::GetAnimPoseAtTime(AnimSequence, Time, EvaluationOptions, AnimPose);
				const FTransform& LastBoneTransform = UAnimPoseExtensions::GetBonePose(AnimPose, CustomExtractBone, EAnimPoseSpaces::World);

				UAnimPoseExtensions::GetAnimPoseAtTime(AnimSequence, Time + SampleInterval, EvaluationOptions, AnimPose);
				const FTransform& CurrentBoneTransform = UAnimPoseExtensions::GetBonePose(AnimPose, CustomExtractBone, EAnimPoseSpaces::World);

				RootMotionDelta = CurrentBoneTransform.GetRelativeTransform(LastBoneTransform);
			}

			Time = FMath::Clamp(Time + SampleInterval, 0.f, AnimLength);

			LastRootMotion = bIsAdditiveCurve ? RootMotionDelta : RootMotionDelta * LastRootMotion;

			Result.UpdateOrAddKey(LastRootMotion, Time);
		}
		return Result;
	}

	static void OverrideAnimBoneMotion(UAnimSequence* Animation, const FTransformCurve& NewRootMotion, FName BoneName = NAME_None)
	{
		if (!Animation)
		{
			UE_LOG(LogAnimation, Warning, TEXT("Invalid AnimSequence"));
			return;
		}
		
		const IAnimationDataModel* Model = Animation->GetDataModel();
		if (Model == nullptr)
		{
			UE_LOG(LogAnimation, Error, TEXT("OverrideAnimRootMotion failed. Reason: Invalid Data Model. Animation: %s"), *GetNameSafe(Animation));
			return;
		}

		const USkeleton* Skeleton = Animation->GetSkeleton();
		if (Skeleton == nullptr)
		{
			UE_LOG(LogAnimation, Error, TEXT("OverrideAnimRootMotion failed. Reason: Invalid Skeleton. Animation: %s"), *GetNameSafe(Animation));
			return;
		}

		const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
		if (RefSkeleton.GetNum() == 0)
		{
			UE_LOG(LogAnimation, Error, TEXT("OverrideAnimRootMotion failed. Reason: Ref Skeleton. Animation: %s"), *GetNameSafe(Animation));
			return;
		}
		
		const FName RootBoneName = BoneName.IsValid() ? BoneName : RefSkeleton.GetBoneName(0);
		
		const int32 NumKeys = Model->GetNumberOfKeys();
		if (NumKeys <= 1)
		{
			UE_LOG(LogAnimation, Error, TEXT("OverrideAnimRootMotion failed. Reason: key number is less 2. Animation: %s"), *GetNameSafe(Animation));
			return;
		}
		
		const float SampleInterval = Model->GetFrameRate().AsInterval();
		
		IAnimationDataController& Controller = Animation->GetController();

		const bool bShouldTransact = false;
		Controller.OpenBracket(NSLOCTEXT("RootMotionEditor", "OverrideAnimRootMotion_Bracket", "Override Anim Root Motion"), bShouldTransact);
		
		TArray<FVector> NewRootTranslations;
		TArray<FQuat> NewRootQuats;
		TArray<FVector> NewRootScales;
		NewRootTranslations.Reserve(NumKeys);
		NewRootQuats.Reserve(NumKeys);
		NewRootScales.Reserve(NumKeys);
		for (int32 AnimKey = 0; AnimKey < NumKeys; AnimKey++)
		{
			const FTransform NewTransform = NewRootMotion.Evaluate(AnimKey * SampleInterval, 1.f);
			NewRootTranslations.Add(NewTransform.GetTranslation());
			NewRootQuats.Add(NewTransform.GetRotation());
			NewRootScales.Add(NewTransform.GetScale3D());
		}

		const FInt32Range KeyRangeToSet(0, NumKeys);
		Controller.UpdateBoneTrackKeys(RootBoneName, KeyRangeToSet, NewRootTranslations, NewRootQuats, NewRootScales, bShouldTransact);
		
		Controller.CloseBracket(bShouldTransact);

		Animation->RefreshCacheData();
	}

	
	static FColor GetColorForAxis(EAxis::Type InAxis)
	{
		return FColor::Black;
	}


	static void DrawFlatArrow(class FPrimitiveDrawInterface* PDI,const FVector& Base,const FVector& XAxis,const FVector& YAxis,FColor Color,float Length,int32 Width, const FMaterialRenderProxy* MaterialRenderProxy, uint8 DepthPriority, float Thickness = 0.0f)
	{
		float DistanceFromBaseToHead = Length/3.0f;
		float DistanceFromBaseToTip = DistanceFromBaseToHead*2.0f;
		float WidthOfBase = Width;
		float WidthOfHead = 2*Width;

		FVector ArrowPoints[7];
		//base points
		ArrowPoints[0] = Base - YAxis*(WidthOfBase*.5f);
		ArrowPoints[1] = Base + YAxis*(WidthOfBase*.5f);
		//inner head
		ArrowPoints[2] = ArrowPoints[0] + XAxis*DistanceFromBaseToHead;
		ArrowPoints[3] = ArrowPoints[1] + XAxis*DistanceFromBaseToHead;
		//outer head
		ArrowPoints[4] = ArrowPoints[2] - YAxis*(WidthOfBase*.5f);
		ArrowPoints[5] = ArrowPoints[3] + YAxis*(WidthOfBase*.5f);
		//tip
		ArrowPoints[6] = Base + XAxis*Length;

		//Draw lines
		{
			//base
			PDI->DrawTranslucentLine(ArrowPoints[0], ArrowPoints[1], Color, DepthPriority, Thickness);
			//base sides																 
			PDI->DrawTranslucentLine(ArrowPoints[0], ArrowPoints[2], Color, DepthPriority, Thickness);
			PDI->DrawTranslucentLine(ArrowPoints[1], ArrowPoints[3], Color, DepthPriority, Thickness);
			//head base																	 
			PDI->DrawTranslucentLine(ArrowPoints[2], ArrowPoints[4], Color, DepthPriority, Thickness);
			PDI->DrawTranslucentLine(ArrowPoints[3], ArrowPoints[5], Color, DepthPriority, Thickness);
			//head sides																 
			PDI->DrawTranslucentLine(ArrowPoints[4], ArrowPoints[6], Color, DepthPriority, Thickness);
			PDI->DrawTranslucentLine(ArrowPoints[5], ArrowPoints[6], Color, DepthPriority, Thickness);

		}

		if (MaterialRenderProxy != nullptr)
		{
			FDynamicMeshBuilder MeshBuilder(PDI->View->GetFeatureLevel());

			//Compute vertices for base circle.
			for(int32 i = 0; i< 7; ++i)
			{
				FDynamicMeshVertex MeshVertex;
				MeshVertex.Position = (FVector3f)ArrowPoints[i];
				MeshVertex.Color = Color;
				MeshVertex.TextureCoordinate[0] = FVector2f(0.0f, 0.0f);;
				MeshVertex.SetTangents(FVector3f(XAxis^YAxis), (FVector3f)YAxis, (FVector3f)XAxis);
				MeshBuilder.AddVertex(MeshVertex); //Add bottom vertex
			}

			//Add triangles / double sided
			{
				MeshBuilder.AddTriangle(0, 2, 1); //base
				MeshBuilder.AddTriangle(0, 1, 2); //base
				MeshBuilder.AddTriangle(1, 2, 3); //base
				MeshBuilder.AddTriangle(1, 3, 2); //base
				MeshBuilder.AddTriangle(4, 5, 6); //head
				MeshBuilder.AddTriangle(4, 6, 5); //head
			}

			MeshBuilder.Draw(PDI, FMatrix::Identity, MaterialRenderProxy, DepthPriority, 0.0f);
		}
	}


	static void DrawCoordinateSystem(FPrimitiveDrawInterface* PDI, const FTransform& Transform, const float Thickness, const float Length, const float DepthBias, const bool bScreenSpace, uint8 Alpha)
	{
		const FVector Location = Transform.GetLocation();
		const FVector AxisX = Transform.GetUnitAxis(EAxis::X) * Length;
		const FVector AxisY = Transform.GetUnitAxis(EAxis::Y) * Length;
		const FVector AxisZ = Transform.GetUnitAxis(EAxis::Z) * Length;
		PDI->DrawTranslucentLine(Location, Location + AxisX, FColor::Red.WithAlpha(Alpha), SDPG_World, 1.0f, DepthBias, bScreenSpace);
		PDI->DrawTranslucentLine(Location, Location + AxisY, FColor::Green.WithAlpha(Alpha), SDPG_World, 1.0f, DepthBias, bScreenSpace);
		PDI->DrawTranslucentLine(Location, Location + AxisZ, FColor::Blue.WithAlpha(Alpha), SDPG_World, 1.0f, DepthBias, bScreenSpace);
	}
}
