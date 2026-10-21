// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseSittableChair.generated.h"

/// <summary>
/// This class is the chair NPC's can sit on
/// can do with a better name.
/// </summary>

class ACharacter;

UCLASS()
class GRADUATIONPROJRESIT_API ABaseSittableChair : public AActor
{
	GENERATED_BODY()
	
public:	

	ABaseSittableChair();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	UPROPERTY()
	bool bSeatIsOccupied = false;

	UPROPERTY()
	AActor* OccupyingActor = nullptr;

public:

	UFUNCTION()
	bool GetIsSeatOccupied();

	UFUNCTION()
	bool SitActorOnChair(AActor* ActorToSitOnChair);

};
