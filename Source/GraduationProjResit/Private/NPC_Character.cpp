// Fill out your copyright notice in the Description page of Project Settings.

#include "NPC_Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HyperloopEventsSubsystem.h"
#include "../Public/Actors/BaseHyperloopPod.h"
#include "BaseSittableChair.h"
#include "NPC_Waypoint.h"

ANPC_Character::ANPC_Character()
{
	PrimaryActorTick.bCanEverTick = true;

	NPCStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCStaticMesh"));
	NPCStaticMesh->SetupAttachment(GetRootComponent());

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCharacterMovement()->bOrientRotationToMovement = false;

	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void ANPC_Character::BeginPlay()
{
	Super::BeginPlay();
	
	if (UWorld* World = GetWorld())
	{
		if (UHyperloopEventsSubsystem* EventsSubsystem = World->GetSubsystem<UHyperloopEventsSubsystem>())
		{
			EventsSubsystem->OnAcceptingPassengers.AddDynamic(this, &ANPC_Character::HandleAcceptingPassengers);
		}
	}
}

void ANPC_Character::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UHyperloopEventsSubsystem* EventsSubsystem = World->GetSubsystem<UHyperloopEventsSubsystem>())
		{
			EventsSubsystem->OnAcceptingPassengers.RemoveDynamic(this, &ANPC_Character::HandleAcceptingPassengers);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ANPC_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsNavigating)
	{
		if (CurrentWayPoint == nullptr) CurrentWayPoint = GetNextWayPoint();

		if (CurrentWayPoint != nullptr)
		{
			FVector TargetLocation = CurrentWayPoint->GetActorLocation();
			FVector CurrentLocation = GetActorLocation();
			CurrentLocation.Z = 0.0f;
			TargetLocation.Z = 0.0f;

			if (FVector::DistSquared(TargetLocation, CurrentLocation) < (WaypointReachedRadius * WaypointReachedRadius))
			{
				// We are close enough to the waypoint so we want to grab a new waypoint to navigate to
				CurrentWayPoint = GetNextWayPoint();

				InterpedRotationalModifier = MinimumInterpedRotationalModifier;
			}
			else
			{
				// waypoint active, we arent close enough so we want to navigate towards it

				InterpedRotationalModifier = FMath::FInterpTo(InterpedRotationalModifier, 1.0f, DeltaTime, RotationChangeInterpSpeed);

				FTransform CurrentTransform = GetActorTransform();
				FTransform DestinationTransform = CurrentWayPoint->GetActorTransform();

				FVector WorldDirection = DestinationTransform.GetLocation() - CurrentTransform.GetLocation();
				WorldDirection.Z = 0.0f;
				WorldDirection.Normalize();

				FVector LocalDirection = CurrentTransform.InverseTransformVector(WorldDirection);

				float RotationalOffset = FMath::RadiansToDegrees(FMath::Atan2(LocalDirection.Y, LocalDirection.X));

				float MaxRotationThisTick = RotationChangeSpeed * DeltaTime;

				if (MaxRotationThisTick > FMath::Abs(RotationalOffset)) MaxRotationThisTick = FMath::Abs(RotationalOffset);

				FRotator NewRotation = GetActorRotation();
				NewRotation.Yaw += (FMath::Sign(RotationalOffset) * MaxRotationThisTick) * InterpedRotationalModifier;
				SetActorRotation(NewRotation);

				if (FMath::Abs(RotationalOffset) < TargetDestinationFacingThreshold)
				{
					AddMovementInput(GetActorForwardVector(), 1.0f);
				}
			}
		}
		else
		{
			// we are done navigating and have arrived at our destination
			bIsNavigating = false;

			UE_LOG(
				LogTemp,
				Log,
				TEXT("[%s] NPC Has successfully navigated to their target"),
				*GetName()
			);

			if (IsValid(TargetHyperloopPod))
			{
				const TArray<ABaseSittableChair*> Chairs = TargetHyperloopPod->GetAllSittableChairs();

				ABaseSittableChair* ClosestChair = nullptr;
				float ClosestDistSq = TNumericLimits<float>::Max();
				const FVector NPCLocation = GetActorLocation();

				for (ABaseSittableChair* Chair : Chairs)
				{
					if (IsValid(Chair) && !Chair->GetIsSeatOccupied())
					{
						const float DistSq = FVector::DistSquared(NPCLocation, Chair->GetActorLocation());
						if (DistSq < ClosestDistSq)
						{
							ClosestDistSq = DistSq;
							ClosestChair = Chair;
						}
					}
				}

				if (IsValid(ClosestChair))
				{
					if (ClosestChair->SitActorOnChair(this))
					{
						UE_LOG(
							LogTemp,
							Log,
							TEXT("[%s] NPC found suitable chair to sit on: [%s]"),
							*GetName(),
							*ClosestChair->GetName()
						);

						bIsSitting = true;
					}
					else
					{
						UE_LOG(
							LogTemp,
							Log,
							TEXT("[%s] NPC failed to sit on chair: [%s]"),
							*GetName(),
							*ClosestChair->GetName()
						);
					}
				}
			}
		}

	}
}

// This class doesnt really need this, but for now im keeping this in
void ANPC_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// handler for when the pod opened its doors to start the npc's walking in
void ANPC_Character::HandleAcceptingPassengers()
{
	UE_LOG(
		LogTemp,
		Log,
		TEXT("[%s] NPC Has Received broadcast and has started navigating"),
		*GetName()
	);

	GetWorldTimerManager().SetTimer(
		NavigationStartDelayTimer,
		this,
		&ANPC_Character::StartNavigation,
		NavigationStartDelaySeconds,
		false
	);
}

void ANPC_Character::StartNavigation()
{
	bIsNavigating = true;
}

ANPC_Waypoint* ANPC_Character::GetNextWayPoint()
{
	if (NavigationWaypointStack.Num() < 1) return nullptr;

	UE_LOG(
		LogTemp, 
		Log, 
		TEXT("[%s] NPC Grabbing next destination. Amount of waypoints in stack: %d"),
		*GetName(),
		NavigationWaypointStack.Num()
	);

	return NavigationWaypointStack.Pop().Get();
}
