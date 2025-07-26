// Fill out your copyright notice in the Description page of Project Settings.


#include "RMEPreviewScene.h"

#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"

FRMEPreviewScene::FRMEPreviewScene(ConstructionValues CVs) : FAdvancedPreviewScene(CVs)
{
	// Disable killing actors outside of the world
	AWorldSettings* WorldSettings = GetWorld()->GetWorldSettings(true);
	WorldSettings->bEnableWorldBoundsChecks = false;

	// Spawn an owner for FloorMeshComponent so CharacterMovementComponent can detect it as a valid floor and slide 
	// along it
	{
		AActor* FloorActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FTransform());
		check(FloorActor);

		static const FString NewName = FString(TEXT("FloorComponent"));
		FloorMeshComponent->Rename(*NewName, FloorActor);

		FloorActor->SetRootComponent(FloorMeshComponent);
	}
}

void FRMEPreviewScene::Tick(float InDeltaTime)
{
	FAdvancedPreviewScene::Tick(InDeltaTime);

	// Trigger Begin Play in this preview world.
	// This is needed for the CharacterMovementComponent to be able to switch to falling mode. 
	// See: UCharacterMovementComponent::StartFalling
	if (PreviewWorld && !PreviewWorld->GetBegunPlay())
	{
		for (FActorIterator It(PreviewWorld); It; ++It)
		{
			It->DispatchBeginPlay();
		}

		PreviewWorld->SetBegunPlay(true);
	}

	GetWorld()->Tick(LEVELTICK_All, InDeltaTime);
}
