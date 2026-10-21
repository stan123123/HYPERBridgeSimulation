#include "Actors/BaseHyperloopPod.h"
#include "PlayerCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "BaseSittableChair.h"
#include "GameAudioSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "HyperloopEventsSubsystem.h"
#include "../MyProjectSettings.h"

ABaseHyperloopPod::ABaseHyperloopPod()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	HazardHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("HazardHitbox"));
	HazardHitbox->SetupAttachment(RootComponent);
	HazardHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	EntranceHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("EntranceHitbox"));
	EntranceHitbox->SetupAttachment(RootComponent);
	EntranceHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PodMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PodMesh"));
	PodMesh->SetupAttachment(RootComponent);

	DoorSoundsSource = CreateDefaultSubobject<USceneComponent>(TEXT("DoorSoundsSource"));
	DoorSoundsSource->SetupAttachment(RootComponent);
}

void ABaseHyperloopPod::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!UMyProjectSettings::GetHyperloopPodTypeData(DataRowName, PodData))
	{
		const FString ErrorMsg = FString::Printf(
			TEXT("ABaseHyperloopPod [%s]: FAILED to find HyperloopPodType row '%s'. Assign a valid DataRowName!"),
			*GetName(), *DataRowName.ToString());

		UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMsg);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, ErrorMsg);
		}
		return;
	}

	if (USkeletalMesh* LoadedMesh = PodData.PodSkeletalMesh.LoadSynchronous())
	{
		PodMesh->SetSkeletalMesh(LoadedMesh);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ABaseHyperloopPod [%s]: Row '%s' found but PodSkeletalMesh is unset."),
			*GetName(), *DataRowName.ToString());
	}
}

void ABaseHyperloopPod::BeginPlay()
{
	Super::BeginPlay();

	// this lazy loads the sounds into memory so when we get them later, there isnt a possibility of a delay
	PodData.DoorOpenSound.LoadSynchronous();
	PodData.DoorCloseSound.LoadSynchronous();

	EntranceHitbox->OnComponentBeginOverlap.AddDynamic(this, &ABaseHyperloopPod::OnEntranceHitboxBeginOverlap);

	if (DestinationStack.Num() < 1)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("WARNING: Hyperloop pod's destination stack array is < 1 at beginplay"),
			this
		);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("Hyperloop Pod's Destinations at beginplay: %d"),
			DestinationStack.Num()
		);
	}

	SetState(EHyperloopPodState::Traveling);
}

void ABaseHyperloopPod::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == EHyperloopPodState::Traveling)
	{
		TickTraveling(DeltaTime);
	}
}

void ABaseHyperloopPod::PushDestination(AActor* Destination)
{
	if (Destination)
	{
		DestinationStack.Push(Destination);
	}
}

AActor* ABaseHyperloopPod::GetCurrentDestination() const
{
	return (DestinationStack.Num() > 0) ? DestinationStack.Last().Get() : nullptr;
}

void ABaseHyperloopPod::SetState(EHyperloopPodState NewState)
{
	CurrentState = NewState;

	switch (CurrentState)
	{
	case EHyperloopPodState::Traveling:
		HazardHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		EntranceHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CurrentSpeed = 0.0f;
		CurrentGForce = 0.0f;
		break;

	case EHyperloopPodState::DoorsOpening:
		
		BeginDoorsOpening();
		break;

	case EHyperloopPodState::BoardingCooldown:
		EntranceHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		GetWorldTimerManager().SetTimer(BoardingCooldownTimerHandle, this,
			&ABaseHyperloopPod::OnBoardingCooldownExpired, BoardingCooldownDuration, false);
		break;

	case EHyperloopPodState::ReadyForDeparture:
		break;

	case EHyperloopPodState::DoorsClosing:
		HazardHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		EntranceHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BeginDoorsClosing();
		break;
	}
}

void ABaseHyperloopPod::TickTraveling(float DeltaTime)
{
	AActor* Target = GetCurrentDestination();
	if (!Target) return;

	const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	const float DistanceToTarget = ToTarget.Size();

	DistanceRemainingToDestination = DistanceToTarget;

	if (DistanceToTarget <= ArrivalMarginCm)
	{
		SetState(EHyperloopPodState::DoorsOpening);
		return;
	}

	const float MaxAccelCmSS = FMath::Max(PodData.GForceAcceleration * GForceToCmPerSecSq, KINDA_SMALL_NUMBER);
	const float BaseStoppingDistance = (CurrentSpeed * CurrentSpeed) / (2.0f * MaxAccelCmSS);

	const float RampCorrectionCm = (InterpConvergence99PctFactor / FMath::Max(PodData.InterpSpeedToAcceleration, KINDA_SMALL_NUMBER)) * CurrentSpeed;

	const float StoppingDistance = BaseStoppingDistance + RampCorrectionCm;
	const bool bShouldDecelerate = StoppingDistance > DistanceToTarget;

	const float TargetGForce = bShouldDecelerate ? -PodData.GForceAcceleration : PodData.GForceAcceleration;
	CurrentGForce = FMath::FInterpTo(CurrentGForce, TargetGForce, DeltaTime, PodData.InterpSpeedToAcceleration);

	const float SignedAccelCmSS = CurrentGForce * GForceToCmPerSecSq;
	CurrentSpeed = FMath::Clamp(CurrentSpeed + SignedAccelCmSS * DeltaTime, 0.0f, PodData.MaxSpeed);

	const FVector Direction = ToTarget.GetSafeNormal();
	const float MoveDistance = FMath::Min(CurrentSpeed * DeltaTime, DistanceToTarget);
	SetActorLocation(GetActorLocation() + Direction * MoveDistance);
}

void ABaseHyperloopPod::BeginDoorsOpening()
{
	if (UHyperloopEventsSubsystem* Events = GetWorld()->GetSubsystem<UHyperloopEventsSubsystem>())
	{
		Events->OnDoorsOpening.Broadcast();
	}

	if (USoundBase* OpenSound = PodData.DoorOpenSound.LoadSynchronous())
	{
		float FinalSFXVolume = 1.0f;
		if (UGameAudioSubsystem* AudioSys = UGameAudioSubsystem::Get(this))
		{
			FinalSFXVolume = AudioSys->SFXVolume;
		}

		UE_LOG(
			LogTemp, 
			Log, 
			TEXT("ABaseHyperloopPod [%s]: Playing Door Open Sound | SFX Volume: %.2f"),
			*GetName(),
			FinalSFXVolume
		);

		UGameplayStatics::PlaySoundAtLocation(
			this,
			OpenSound,
			DoorSoundsSource->GetComponentLocation(),
			FinalSFXVolume,
			1.0f         
		);
	}

	if (UAnimMontage* OpenMontage = PodData.OpenAnimation.LoadSynchronous())
	{
		PodMesh->PlayAnimation(OpenMontage, false);

		GetWorldTimerManager().SetTimer(DoorAnimTimerHandle, this,
			&ABaseHyperloopPod::NotifyDoorsOpened, OpenMontage->GetPlayLength(), false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ABaseHyperloopPod [%s]: Row '%s' has no OpenAnimation assigned. Skipping to opened state."),
			*GetName(), 
			*DataRowName.ToString()
		);
		NotifyDoorsOpened();
	}
}

void ABaseHyperloopPod::BeginDoorsClosing()
{
	if (UHyperloopEventsSubsystem* Events = GetWorld()->GetSubsystem<UHyperloopEventsSubsystem>())
	{
		Events->OnDoorsClosing.Broadcast();
	}

	if (USoundBase* CloseSound = PodData.DoorCloseSound.LoadSynchronous())
	{
		float FinalSFXVolume = 1.0f;
		if (UGameAudioSubsystem* AudioSys = UGameAudioSubsystem::Get(this))
		{
			FinalSFXVolume = AudioSys->SFXVolume;
		}

		UE_LOG(LogTemp, Log, TEXT("ABaseHyperloopPod [%s]: Playing Door Close Sound | SFX Volume: %.2f"),
			*GetName(),
			FinalSFXVolume
		);

		UGameplayStatics::PlaySoundAtLocation(
			this,
			CloseSound,
			DoorSoundsSource->GetComponentLocation(),
			FinalSFXVolume,
			1.0f
		);
	}

	if (UAnimMontage* CloseMontage = PodData.CloseAnimation.LoadSynchronous())
	{
		PodMesh->PlayAnimation(CloseMontage, false);

		GetWorldTimerManager().SetTimer(DoorAnimTimerHandle, this,
			&ABaseHyperloopPod::NotifyDoorsClosed, CloseMontage->GetPlayLength(), false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ABaseHyperloopPod [%s]: Row '%s' has no CloseAnimation assigned. Skipping to closed state."),
			*GetName(), 
			*DataRowName.ToString()
		);
		NotifyDoorsClosed();
	}
}

void ABaseHyperloopPod::NotifyDoorsOpened()
{
	HazardHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetState(EHyperloopPodState::BoardingCooldown);

	if (UHyperloopEventsSubsystem* Events = GetWorld()->GetSubsystem<UHyperloopEventsSubsystem>())
	{
		Events->BroadcastAcceptingPassengers();

		UE_LOG(
			LogTemp,
			Log,
			TEXT("[%s] HyperloopPod has Broadcast BroadcastAcceptingPassengers"),
			*GetName()
		);
	}
}

void ABaseHyperloopPod::NotifyDoorsClosed()
{
	if (DestinationStack.Num() > 0)
	{
		DestinationStack.Pop();
	}
	SetState(EHyperloopPodState::Traveling);

	if (AActor* Target = GetCurrentDestination())
	{
		TotalDistanceToDestination = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
		DistanceRemainingToDestination = TotalDistanceToDestination;
	}
	else
	{
		TotalDistanceToDestination = 0.0f;
		DistanceRemainingToDestination = 0.0f;
	}
}

void ABaseHyperloopPod::OnBoardingCooldownExpired()
{
	if (DestinationStack.Num() < 2)
	{
		return;
	}

	TArray<AActor*> OverlappingPlayers;
	EntranceHitbox->GetOverlappingActors(OverlappingPlayers, APlayerCharacterBase::StaticClass());

	if (OverlappingPlayers.Num() > 0)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[%s] Player detected in entrance when boarding cooldown expired. Closing doors."),
			*GetName()
		);

		SetState(EHyperloopPodState::DoorsClosing);
	}
	else
	{
		SetState(EHyperloopPodState::ReadyForDeparture);
	}
}

void ABaseHyperloopPod::OnEntranceHitboxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (DestinationStack.Num() < 2)
	{
		return;
	}

	if (OtherActor)
	{
		UE_LOG(
			LogTemp, 
			Log, 
			TEXT("Hyperloop entrance hitbox touched by actor of type: %s"),
			*OtherActor->GetClass()->GetName()
		);
	}

	if (APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(OtherActor))
	{
		if (CurrentState == EHyperloopPodState::ReadyForDeparture)
		{
			SetState(EHyperloopPodState::DoorsClosing);

			UE_LOG(
				LogTemp, 
				Log, 
				TEXT("Hyperloop departure triggered, closing doors!")
			);
		}
	}
}

TArray<ABaseSittableChair*> ABaseHyperloopPod::GetAllSittableChairs()
{
	TArray<ABaseSittableChair*> FoundChairs;

	TArray<UChildActorComponent*> ChildComponents;
	GetComponents<UChildActorComponent>(ChildComponents);

	for (UChildActorComponent* ChildComp : ChildComponents)
	{
		if (IsValid(ChildComp))
		{
			if (ABaseSittableChair* Chair = Cast<ABaseSittableChair>(ChildComp->GetChildActor()))
			{
				FoundChairs.Add(Chair);
			}
		}
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[%s] Hyperloop Pod Returning sittable chairs, Amount returned: %i"),
		*GetName(),
		FoundChairs.Num()
	);

	return FoundChairs;
}