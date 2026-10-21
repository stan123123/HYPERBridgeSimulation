// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Containers/RingBuffer.h"
#include "CarManagerBase.generated.h"

/// <summary>
/// This class is responsible for managing hundreds of cars driving along a fixed road on the map as background props.
/// It does this by pooling the objects as ISM's at beginplay and moving them at a fixed speed of 100km/h
/// </summary>

class UStaticMesh;
class UInstancedStaticMeshComponent;

struct FCarSoA
{
	TRingBuffer<float> DistanceTraveled;
	TRingBuffer<uint8> VariantIndex;
	TRingBuffer<int32> InstanceIndex;

	int32 Num() const { return DistanceTraveled.Num(); }

	void Add(float Distance, uint8 Variant, int32 Instance)
	{
		DistanceTraveled.Add(Distance);
		VariantIndex.Add(Variant);
		InstanceIndex.Add(Instance);
	}

	void PopOldest()
	{
		DistanceTraveled.PopFront();
		VariantIndex.PopFront();
		InstanceIndex.PopFront();
	}
};

UCLASS()
class GRADUATIONPROJRESIT_API ACarManagerBase : public AActor
{
	GENERATED_BODY()

public:
	ACarManagerBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	// 100 km/h in cm/s
	static constexpr float CarSpeed = 2777.7f;

	FCarSoA ManagedCars;

	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> VariantISMs;

	// Per-variant stack of unused instance slots
	TArray<TArray<int32>> FreeInstanceSlots;

	FVector PathOrigin = FVector::ZeroVector;
	FVector PathDirection = FVector::ForwardVector;
	FQuat PathRotation = FQuat::Identity;
	int32 PoolSize = 0;
	float SpawnTimer = 0.0f;
	bool bPoolReady = false;

	UPROPERTY(EditInstanceOnly, Category = "Vehicle spawning")
	TArray<TObjectPtr<UStaticMesh>> DifferentCarsToSpawn;

	UPROPERTY(EditInstanceOnly, Category = "Vehicle spawning", meta = (ClampMin = "0.1"))
	float MinSpawnCooldown = 3.0f;

	UPROPERTY(EditInstanceOnly, Category = "Vehicle spawning", meta = (ClampMin = "0.1"))
	float MaxSpawnCooldown = 6.0f;

	UPROPERTY(EditInstanceOnly, Category = "Vehicle spawning", meta = (ClampMin = "1"))
	int32 MaxCarsLimit = 100;

	UPROPERTY(EditInstanceOnly, Category = "Vehicle spawning", meta = (ClampMin = "1.0"))
	float CarDespawnDistanceCM = 1000.0f;

	void PrePopulateSOAWithCars();
	void SpawnNewCar(float StartDistance);
	void DespawnOldestCar();
	FTransform MakeHiddenTransform() const;
};