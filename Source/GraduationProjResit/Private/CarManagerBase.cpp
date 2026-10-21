// Fill out your copyright notice in the Description page of Project Settings.


#include "CarManagerBase.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"

ACarManagerBase::ACarManagerBase()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ACarManagerBase::BeginPlay()
{
	Super::BeginPlay();

	const int32 NumVariants = DifferentCarsToSpawn.Num();
	if (NumVariants == 0 || NumVariants > 255)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: needs 1-255 meshes in DifferentCarsToSpawn"), *GetName());
		SetActorTickEnabled(false);
		return;
	}
	for (const TObjectPtr<UStaticMesh>& Mesh : DifferentCarsToSpawn)
	{
		if (!Mesh)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: null mesh in DifferentCarsToSpawn"), *GetName());
			SetActorTickEnabled(false);
			return;
		}
	}

	PathOrigin = GetActorLocation();
	PathDirection = GetActorForwardVector();
	PathRotation = GetActorQuat();

	const float MinSpacing = MinSpawnCooldown * CarSpeed;
	PoolSize = FMath::Clamp(FMath::FloorToInt(CarDespawnDistanceCM / MinSpacing) + 1, 1, MaxCarsLimit);

	VariantISMs.Reserve(NumVariants);
	FreeInstanceSlots.SetNum(NumVariants);
	const FTransform Hidden = MakeHiddenTransform();

	for (int32 V = 0; V < NumVariants; ++V)
	{
		UInstancedStaticMeshComponent* ISM = NewObject<UInstancedStaticMeshComponent>(this);
		ISM->SetStaticMesh(DifferentCarsToSpawn[V]);
		ISM->SetMobility(EComponentMobility::Movable);
		ISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ISM->SetGenerateOverlapEvents(false);
		ISM->SetCanEverAffectNavigation(false);
		ISM->SetupAttachment(GetRootComponent());
		ISM->RegisterComponent();

		FreeInstanceSlots[V].Reserve(PoolSize);
		for (int32 i = 0; i < PoolSize; ++i)
		{
			FreeInstanceSlots[V].Add(ISM->AddInstance(Hidden, true));
		}
		VariantISMs.Add(ISM);
	}

	ManagedCars.DistanceTraveled.Reserve(PoolSize);
	ManagedCars.VariantIndex.Reserve(PoolSize);
	ManagedCars.InstanceIndex.Reserve(PoolSize);

	bPoolReady = true;
	PrePopulateSOAWithCars();
}


/// <summary>
/// Do not EVER!!!! call this before beginplay (like in OnConstruct, the Constructor or OnRegister) it will clash with unreal's component registry system and nuke the whole actor.
/// you have been warned.
/// </summary>
void ACarManagerBase::PrePopulateSOAWithCars()
{
	const float MaxCooldown = FMath::Max(MinSpawnCooldown, MaxSpawnCooldown);

	const float NextGap = FMath::FRandRange(MinSpawnCooldown, MaxCooldown);
	const float Elapsed = FMath::FRandRange(0.0f, NextGap);
	SpawnTimer = NextGap - Elapsed;

	TArray<float> Distances;
	Distances.Reserve(PoolSize);

	float Distance = Elapsed * CarSpeed;
	while (Distances.Num() < PoolSize && Distance < CarDespawnDistanceCM)
	{
		Distances.Add(Distance);
		Distance += FMath::FRandRange(MinSpawnCooldown, MaxCooldown) * CarSpeed;
	}

	for (int32 i = Distances.Num() - 1; i >= 0; --i)
	{
		SpawnNewCar(Distances[i]);
	}
}

void ACarManagerBase::SpawnNewCar(float StartDistance)
{
	if (ManagedCars.Num() >= PoolSize)
	{
		return;
	}

	const int32 Variant = FMath::RandRange(0, VariantISMs.Num() - 1);
	TArray<int32>& Free = FreeInstanceSlots[Variant];
	if (Free.IsEmpty())
	{
		return;
	}

	const int32 Slot = Free.Pop();
	VariantISMs[Variant]->UpdateInstanceTransform(
		Slot,
		FTransform(
			PathRotation, 
			PathOrigin + PathDirection * StartDistance
		),
		true, true, true
	);

	ManagedCars.Add(
		StartDistance, 
		static_cast<uint8>(Variant), 
		Slot
	);
}

void ACarManagerBase::DespawnOldestCar()
{
	const uint8 Variant = ManagedCars.VariantIndex[0];
	const int32 Slot = ManagedCars.InstanceIndex[0];

	VariantISMs[Variant]->UpdateInstanceTransform(
		Slot, MakeHiddenTransform(),
		true, 
		true,
		true
	);

	FreeInstanceSlots[Variant].Add(Slot);
	ManagedCars.PopOldest();
}

FTransform ACarManagerBase::MakeHiddenTransform() const
{
	return FTransform(
		FQuat::Identity, 
		PathOrigin, 
		FVector(KINDA_SMALL_NUMBER)
	);
}

void ACarManagerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bPoolReady)
	{
		return;
	}

	const float Step = CarSpeed * DeltaTime;
	TArray<bool, TInlineAllocator<16>> Dirty;
	Dirty.Init(false, VariantISMs.Num());

	for (int32 i = 0; i < ManagedCars.Num(); ++i)
	{
		float& Distance = ManagedCars.DistanceTraveled[i];
		Distance += Step;

		const uint8 Variant = ManagedCars.VariantIndex[i];
		VariantISMs[Variant]->UpdateInstanceTransform(
			ManagedCars.InstanceIndex[i],
			FTransform(
				PathRotation, 
				PathOrigin + 
				PathDirection * 
				Distance
			),
			true, 
			false,
			true
		);
		Dirty[Variant] = true;
	}

	for (int32 V = 0; V < Dirty.Num(); ++V)
	{
		if (Dirty[V])
		{
			VariantISMs[V]->MarkRenderStateDirty();
		}
	}

	while (ManagedCars.Num() > 0 && ManagedCars.DistanceTraveled[0] >= CarDespawnDistanceCM)
	{
		DespawnOldestCar();
	}

	SpawnTimer -= DeltaTime;
	if (SpawnTimer <= 0.0f && ManagedCars.Num() < PoolSize)
	{
		SpawnNewCar(0.0f);
		SpawnTimer = FMath::FRandRange(
			MinSpawnCooldown, 
			FMath::Max(
				MinSpawnCooldown, 
				MaxSpawnCooldown
			)
		);
	}
}