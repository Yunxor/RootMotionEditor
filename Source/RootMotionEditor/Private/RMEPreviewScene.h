// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedPreviewScene.h"

/**
 * 
 */
class FRMEPreviewScene : public FAdvancedPreviewScene
{
public:
	FRMEPreviewScene(ConstructionValues CVs);
	virtual ~FRMEPreviewScene() {}

	virtual void Tick(float InDeltaTime) override;

};
