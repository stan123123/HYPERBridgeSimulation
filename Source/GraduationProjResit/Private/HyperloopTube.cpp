// Fill out your copyright notice in the Description page of Project Settings.

#include "HyperloopTube.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "HyperloopEventsSubsystem.h"
#include "Engine/World.h"

AHyperloopTube::AHyperloopTube()
{
	PrimaryActorTick.bCanEverTick = true;

	TubeRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TubeRoot"));
	RootComponent = TubeRoot;
}

void AHyperloopTube::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void AHyperloopTube::RebuildTubeSegments() 
{
	for (UStaticMeshComponent* Segment : TubeSegmentComponents) 
	{ 
		if (Segment) 
		{ 
			Segment->DestroyComponent(); 
		} 
	} 
	
	TubeSegmentComponents.Empty(); 
	
	for (UStaticMeshComponent* CutSegment : CutThroughSegmentComponents) 
	{
		if (CutSegment) 
		{ 
			CutSegment->DestroyComponent(); 
		} 
	} 
	
	CutThroughSegmentComponents.Empty(); 
	
	if (totalTubeSegments <= 0) { return; }
	if (!TubeMeshClosed) { return; }
	if (tubeSegmentLengthCM <= 0.f) { return; }

	for (int32 i = 0; i < totalTubeSegments; ++i)
	{
		const FName CompName = *FString::Printf(TEXT("TubeSegment_%d"), i);
		UStaticMeshComponent* SegmentMesh = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), CompName);

		if (SegmentMesh)
		{
			SegmentMesh->SetStaticMesh(TubeMeshClosed);
			SegmentMesh->SetupAttachment(RootComponent);
			SegmentMesh->SetRelativeLocation(FVector(i * tubeSegmentLengthCM, 0.f, 0.f));
			SegmentMesh->SetRelativeRotation(FRotator::ZeroRotator);

			SegmentMesh->RegisterComponent();
			SegmentMesh->UpdateBounds();
			SegmentMesh->SetBoundsScale(2.0f);

			TubeSegmentComponents.Add(SegmentMesh);
		}
	}

	if (!TubeMeshClosedCutThrough) { return; }

	for (int32 i = 0; i < totalTubeSegments; ++i)
	{
		const FName CompName = *FString::Printf(TEXT("CutThroughTubeSegment_%d"), i);
		UStaticMeshComponent* CutSegmentMesh = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), CompName);

		if (CutSegmentMesh)
		{
			CutSegmentMesh->SetStaticMesh(TubeMeshClosedCutThrough);
			CutSegmentMesh->SetupAttachment(RootComponent);
			CutSegmentMesh->SetRelativeLocation(FVector(i * tubeSegmentLengthCM, 0.f, 0.f));
			CutSegmentMesh->SetRelativeRotation(FRotator::ZeroRotator);

			CutSegmentMesh->RegisterComponent();
			CutSegmentMesh->UpdateBounds();
			CutSegmentMesh->SetBoundsScale(2.0f);

			CutThroughSegmentComponents.Add(CutSegmentMesh);
		}
	}

	if (RootComponent)
	{
		RootComponent->UpdateBounds();
	}
}

void AHyperloopTube::SetTubeSeeThrough(bool bMakeSeeThrough)
{
	if (!bCanBecomeTransparent)
	{
		return;
	}

	bIsCurrentlySeeThrough = bMakeSeeThrough;
}

void AHyperloopTube::BeginPlay() {
	Super::BeginPlay(); 

	RebuildTubeSegments();

	TubeSegmentComponents.Empty();
	CutThroughSegmentComponents.Empty();

	TArray<UStaticMeshComponent*> StaticMeshComponents;
	GetComponents<UStaticMeshComponent>(StaticMeshComponents);

	for (UStaticMeshComponent* Component : StaticMeshComponents)
	{
		if (!Component) continue;

		Component->UpdateComponentToWorld();
		Component->UpdateBounds();

		if (Component->GetStaticMesh() == TubeMeshClosed)
		{
			TubeSegmentComponents.Add(Component);
		}
		else if (Component->GetStaticMesh() == TubeMeshClosedCutThrough)
		{
			CutThroughSegmentComponents.Add(Component);
		}
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("BeginPlay - this=%p | Array Num=%d"),
		this,
		TubeSegmentComponents.Num()
	);

	ClosedTubeMIDs.Empty(); 

	if (TubeSegmentComponents.Num() < 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: TubeSegmentComponents array is empty in HyperloopTube!"));
	}
	
	for (UStaticMeshComponent* Segment : TubeSegmentComponents)
	{
		if (!Segment)
		{
			continue;
		}

		const int32 MaterialCount = Segment->GetNumMaterials();

		for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
		{
			UMaterialInstanceDynamic* MID =
				Segment->CreateAndSetMaterialInstanceDynamic(MaterialIndex);

			if (MID)
			{
				MID->SetScalarParameterValue(
					TEXT("FadeOpacity"),
					CurrentOpacity
				);

				ClosedTubeMIDs.Add(MID);
			}
		}
	}
	
	if (UWorld* World = GetWorld()) 
	{ 
		if (UHyperloopEventsSubsystem* EventsSubsystem = World->GetSubsystem<UHyperloopEventsSubsystem>()) 
		{ 
			EventsSubsystem->OnCinematicStart.AddDynamic( this, &AHyperloopTube::HandleCinematicStart ); 
			EventsSubsystem->OnCinematicComplete.AddDynamic( this, &AHyperloopTube::HandleCinematicComplete ); 
		} 
	} 
}

void AHyperloopTube::Tick(float DeltaTime) 
{
	Super::Tick(DeltaTime); 
	
	if (bIsFadingOut && CurrentOpacity > 0.0f) 
	{ 
		CurrentOpacity = FMath::FInterpTo(
			CurrentOpacity, 
			0.0f, 
			DeltaTime, 
			CinematicFadeSpeed
		); 
		
		if (CurrentOpacity < 0.001f) 
		{ 
			CurrentOpacity = 0.0f; 
		} 
		
		for (UMaterialInstanceDynamic* MID : ClosedTubeMIDs) 
		{ 
			if (MID) 
			{ 
				MID->SetScalarParameterValue(TEXT("FadeOpacity"), CurrentOpacity); 
			} 
		} 
	} 
}

void AHyperloopTube::HandleCinematicStart() 
{ 
	if (!bCanBecomeTransparent) { return; } 

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("HandleCinematicStart - this=%p | Array Num=%d"),
		this,
		TubeSegmentComponents.Num()
	);
	
	bIsFadingOut = true; 
	
	SetCutThroughSegmentsVisibility(true);
}

void AHyperloopTube::HandleCinematicComplete()
{
	if (!bCanBecomeTransparent) return;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("HandleCinematicComplete - this=%p | Array Num=%d"),
		this,
		TubeSegmentComponents.Num()
	);

	bIsFadingOut = false;
	CurrentOpacity = 1.0f;

	for (UMaterialInstanceDynamic* MID : ClosedTubeMIDs)
	{
		if (MID)
		{
			MID->SetScalarParameterValue(FName("FadeOpacity"), CurrentOpacity);
		}
	}

	SetCutThroughSegmentsVisibility(false);
}

void AHyperloopTube::SetCutThroughSegmentsVisibility(bool bIsVisible)
{
	for (UStaticMeshComponent* CutSegment : CutThroughSegmentComponents)
	{
		if (CutSegment)
		{
			CutSegment->SetVisibility(bIsVisible);
		}
	}
}