// Fill out your copyright notice in the Description page of Project Settings.


#include "RMEViewModel.h"
#include "AnimPreviewInstance.h"
#include "RMEContext.h"
#include "RMEPreviewScene.h"
#include "Animation/DebugSkelMeshComponent.h"


bool FRootMotionEditorPreviewActor::SetupPreviewActor(UWorld* World, UAnimSequence* InAnimation)
{
	if (InAnimation == nullptr)
	{
		ClearPreviewActor();
		return false;
	}

	USkeleton* Skeleton = InAnimation->GetSkeleton();
	if (!Skeleton)
	{
		UE_LOG(LogRootMotionEditor, Error, TEXT("[FRootMotionEditedPreviewActor::SpawnPreviewActor] Couldn't spawn preview Actor because of missing skeleton in anim asset(%s)"), *GetNameSafe(InAnimation));
		return false;
	}

	AnimAssetPtr = InAnimation;

	UAnimPreviewInstance* AnimInstance;
	if (ActorPtr == nullptr)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		ActorPtr = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Params);
		ActorPtr->SetFlags(RF_Transient);

		UDebugSkelMeshComponent* Mesh = NewObject<UDebugSkelMeshComponent>(ActorPtr.Get());
		Mesh->RegisterComponentWithWorld(World);
		Mesh->SetVisualizeRootMotionMode(EVisualizeRootMotionMode::Trajectory);

		AnimInstance = NewObject<UAnimPreviewInstance>(Mesh);
		Mesh->PreviewInstance = AnimInstance;
		AnimInstance->InitializeAnimation();

		USkeletalMesh* DatabasePreviewMesh = InAnimation->GetPreviewMesh();
		Mesh->SetSkeletalMesh(DatabasePreviewMesh ? DatabasePreviewMesh : Skeleton->GetPreviewMesh(true));
		Mesh->EnablePreview(true, InAnimation);
		
		if (!ActorPtr->GetRootComponent())
		{
			ActorPtr->SetRootComponent(Mesh);
		}
	}
	else
	{
		UDebugSkelMeshComponent* Mesh = GetDebugSkelMeshComponent();
		if (Mesh->GetWorld() != World)
		{
			Mesh->UnregisterComponent();
			Mesh->RegisterComponentWithWorld(World);
		}
		
		AnimInstance = GetAnimPreviewInstance();

		// todo : preview settings control it.
		USkeletalMesh* DatabasePreviewMesh = InAnimation->GetPreviewMesh();
		Mesh->SetSkeletalMesh(DatabasePreviewMesh ? DatabasePreviewMesh : Skeleton->GetPreviewMesh(true));
		Mesh->EnablePreview(true, InAnimation);
	}

	if (AnimInstance != nullptr)
	{
		AnimInstance->SetAnimationAsset(InAnimation, InAnimation->bLoop, 0.f);
		AnimInstance->PlayAnim(InAnimation->bLoop, 0.f);
		AnimInstance->SetPlayRate(0.f);
	}
	else
	{
		UE_LOG(LogRootMotionEditor, Error, TEXT("[FRootMotionEditedPreviewActor::SpawnPreviewActor] Can't found anim instance."));
	}

	UE_LOG(LogRootMotionEditor, Log, TEXT("[FRootMotionEditedPreviewActor::SpawnPreviewActor] Spawned preview Actor: %s"), *GetNameSafe(ActorPtr.Get()));
	return true;
}

void FRootMotionEditorPreviewActor::UpdatePreviewActor(float PlayTime, FRMEViewModel* InViewModel)
{
	UAnimSequenceBase* AnimSeq = AnimAssetPtr.Get();
	if (!IsValid(AnimSeq))
	{
		return;
	}
	
	UAnimPreviewInstance* AnimInstance = GetAnimPreviewInstanceInternal();
	if (!AnimInstance)
	{
		return;
	}

	AnimInstance->SetPosition(PlayTime);
	AnimInstance->SetPlayRate(0.f);

	if (ActorPtr != nullptr && InViewModel != nullptr)
	{
		ActorPtr->SetActorTransform(InViewModel->GetRootMotionTransform(PlayTime));
	}
}

void FRootMotionEditorPreviewActor::ClearPreviewActor()
{
	if (UDebugSkelMeshComponent* Mesh = GetDebugSkelMeshComponent())
	{
		Mesh->UnregisterComponent();
	}
	if (ActorPtr != nullptr)
	{
		ActorPtr->Destroy();
	}
	
	AnimAssetPtr = nullptr;
}

bool FRootMotionEditorPreviewActor::DrawPreviewActor()
{
	return true;
}

void FRootMotionEditorPreviewActor::Destroy()
{
	if (ActorPtr != nullptr)
	{
		ActorPtr->Destroy();
		ActorPtr = nullptr;
	}
}

UAnimPreviewInstance* FRootMotionEditorPreviewActor::GetAnimPreviewInstanceInternal()
{
	if (ActorPtr != nullptr)
	{
		if (UDebugSkelMeshComponent* Mesh = Cast<UDebugSkelMeshComponent>(ActorPtr->GetRootComponent()))
		{
			return Mesh->PreviewInstance.Get();
		}
	}
	return nullptr;
}


UDebugSkelMeshComponent* FRootMotionEditorPreviewActor::GetDebugSkelMeshComponent() const
{
	if (ActorPtr != nullptr)
	{
		return Cast<UDebugSkelMeshComponent>(ActorPtr->GetRootComponent());
	}
	return nullptr;
}

UAnimPreviewInstance* FRootMotionEditorPreviewActor::GetAnimPreviewInstance() const
{
	if (const UDebugSkelMeshComponent* Mesh = GetDebugSkelMeshComponent())
	{
		return Mesh->PreviewInstance.Get();
	}
	return nullptr;
}


void FRMEViewModel::AddReferencedObjects(FReferenceCollector& Collector)
{
}

void FRMEViewModel::Initialize(const TSharedRef<FRMEPreviewScene>& InPreviewScene)
{
	PreviewScenePtr = InPreviewScene;
	ManipulatorTransform = FTransform::Identity;
}

void FRMEViewModel::Tick(float DeltaSeconds)
{
	const float DeltaPlayTime = DeltaSeconds * DeltaTimeMultiplier;
	const float NewPlayTime = FMath::Clamp(PlayTime + DeltaPlayTime, 0.f, MaxPreviewPlayLength);
	const bool bHasTimeChanged = !FMath::IsNearlyEqual(NewPlayTime, PlayTime);
	PlayTime = NewPlayTime;
	PreviewActor.UpdatePreviewActor(PlayTime, this);

	if (bHasTimeChanged && PreviewEditMode != ERMEPreviewEditMode::View && !bManipulatorHasUserOverride)
	{
		SyncManipulatorToCurrentRootMotion();
	}
}

void FRMEViewModel::SetSelectedAnimation(UAnimSequence* InAnimation)
{
	MaxPreviewPlayLength = InAnimation ? InAnimation->GetPlayLength() : 0.f;
	PreviewActor.SetupPreviewActor(GetWorld(), InAnimation);

	// auto change view mode.
	if (RootMotionViewMode == ERMERootMotionViewMode::None)
	{
		RootMotionViewMode = ERMERootMotionViewMode::Asset;
	}

	if (PreviewEditMode != ERMEPreviewEditMode::View && !bManipulatorHasUserOverride)
	{
		SyncManipulatorToCurrentRootMotion();
	}
}


UWorld* FRMEViewModel::GetWorld()
{
	check(PreviewScenePtr.IsValid());
	return PreviewScenePtr.Pin()->GetWorld();
} 

void FRMEViewModel::SetPlayTime(float NewPlayTime, bool bInTickPlayTime)
{
	NewPlayTime = FMath::Clamp(NewPlayTime, MinPreviewPlayLength, MaxPreviewPlayLength);
	DeltaTimeMultiplier = bInTickPlayTime ? DeltaTimeMultiplier : 0.f;

	if (!FMath::IsNearlyEqual(PlayTime, NewPlayTime))
	{
		PlayTime = NewPlayTime;
		PreviewActor.UpdatePreviewActor(PlayTime, this);

		if (PreviewEditMode != ERMEPreviewEditMode::View)
		{
			// An explicit time change means the manipulator should snap to the current frame.
			SyncManipulatorToCurrentRootMotion();
		}
	}
}

TRange<double> FRMEViewModel::GetPlayTimeRange() const
{
	constexpr double ViewRangeSlack = 0.2;
	return TRange<double>(MinPreviewPlayLength - ViewRangeSlack, MaxPreviewPlayLength + ViewRangeSlack);
}

void FRMEViewModel::SetRootMotionViewMode(ERMERootMotionViewMode InType)
{
	RootMotionViewMode = InType;

	if (PreviewEditMode != ERMEPreviewEditMode::View && !bManipulatorHasUserOverride)
	{
		SyncManipulatorToCurrentRootMotion();
	}
}

void FRMEViewModel::SetPreviewEditMode(ERMEPreviewEditMode InType)
{
	PreviewEditMode = InType;

	if (PreviewEditMode != ERMEPreviewEditMode::View)
	{
		PreviewPause();
		SyncManipulatorToCurrentRootMotion();
	}
	else
	{
		ClearManipulatorUserOverride();
	}
}

void FRMEViewModel::SyncManipulatorToCurrentRootMotion()
{
	ManipulatorTransform = GetRootMotionTransform(PlayTime);
	ClearManipulatorUserOverride();
}

void FRMEViewModel::SetManipulatorTransform(const FTransform& InTransform)
{
	ManipulatorTransform = InTransform;
	bManipulatorHasUserOverride = true;
}

void FRMEViewModel::SetManipulatorLocation(const FVector& InLocation)
{
	ManipulatorTransform.SetLocation(InLocation);
	bManipulatorHasUserOverride = true;
}

void FRMEViewModel::AddManipulatorTranslation(const FVector& InTranslation)
{
	PreviewPause();
	ManipulatorTransform.AddToTranslation(InTranslation);
	bManipulatorHasUserOverride = true;
}

void FRMEViewModel::PreviewBackwardEnd()
{
	SetPlayTime(MinPreviewPlayLength, false);
}

void FRMEViewModel::PreviewBackwardStep()
{
	const float NewPlayTime = FMath::Clamp(PlayTime - StepDeltaTime, MinPreviewPlayLength, MaxPreviewPlayLength);
	SetPlayTime(NewPlayTime, false);
}

void FRMEViewModel::PreviewBackward()
{
	DeltaTimeMultiplier = -1.f;
}

void FRMEViewModel::PreviewPause()
{
	DeltaTimeMultiplier = 0.f;
}

void FRMEViewModel::PreviewForward()
{
	DeltaTimeMultiplier = 1.f;
}

void FRMEViewModel::PreviewForwardStep()
{
	const float NewPlayTime = FMath::Clamp(PlayTime + StepDeltaTime, MinPreviewPlayLength, MaxPreviewPlayLength);
	SetPlayTime(NewPlayTime, false);
}

void FRMEViewModel::PreviewForwardEnd()
{
	SetPlayTime(MaxPreviewPlayLength, false);
}

FTransform FRMEViewModel::GetRootMotionTransform(float Time) const
{
	FTransform RootMotionTransform = FTransform::Identity;
	
	switch (GetRootMotionViewMode()) {
	case ERMERootMotionViewMode::None:
		break;
	case ERMERootMotionViewMode::Asset:
		{
			const UAnimSequence* AnimSeq = GetAnimation();
			if (AnimSeq != nullptr)
			{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 6
				FAnimExtractContext ExtractContext(Time, true, {}, false);
				RootMotionTransform = AnimSeq->ExtractRootMotionFromRange(0, Time, ExtractContext);
#else
				RootMotionTransform = AnimSeq->ExtractRootMotionFromRange(0, Time);
#endif
			}
		}
		break;
	case ERMERootMotionViewMode::Editor:
		if (const FRMEContext* Context = FRMEContext::Get())
		{
			RootMotionTransform = Context->GetCurveTransform(Time, 1.f);
		}
		break;
	}

	return RootMotionTransform;
}
