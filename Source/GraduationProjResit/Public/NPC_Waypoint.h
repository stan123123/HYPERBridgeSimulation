// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NPC_Waypoint.generated.h"

/// <summary>
/// Very light object, msotly exists as a type for the NPC to reference as waypoint, besides that doesnt have functionality
/// </summary>

UCLASS()
class GRADUATIONPROJRESIT_API ANPC_Waypoint : public AActor
{
	GENERATED_BODY()
	
public:	
	ANPC_Waypoint();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
