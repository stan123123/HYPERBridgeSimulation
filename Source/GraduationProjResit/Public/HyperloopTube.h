// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HyperloopTube.generated.h"

/// <summary>
/// This class is a class that manages the tubes on the map. it dynamically spawns tubes onbeginplay. It used to spawn in the constructor but due to an in editor re instancing bug, The logic was moved to beginplay
/// It also makes the tube see through by listening to cinematic events
/// </summary>

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

UCLASS()
class GRADUATIONPROJRESIT_API AHyperloopTube : public AActor
{
	GENERATED_BODY()

public:
	AHyperloopTube();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
	int totalTubeSegments = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
	bool bCanBecomeTransparent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UStaticMesh> TubeMeshClosed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UStaticMesh> TubeMeshClosedCutThrough;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
	float tubeSegmentLengthCM = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
	float CinematicFadeSpeed = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "Hyperloop")
	void SetTubeSeeThrough(bool bMakeSeeThrough);

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	void RebuildTubeSegments();

	UFUNCTION()
	void SetCutThroughSegmentsVisibility(bool bIsVisible);

	UFUNCTION()
	void HandleCinematicStart();

	UFUNCTION()
	void HandleCinematicComplete();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"), Category = "Hyperloop")
	TObjectPtr<USceneComponent> TubeRoot;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> TubeSegmentComponents;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> CutThroughSegmentComponents;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> ClosedTubeMIDs;

	bool bIsCurrentlySeeThrough = false;

	bool bIsFadingOut = false;

	float CurrentOpacity = 1.0f;
};