// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseSittableChair.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"

ABaseSittableChair::ABaseSittableChair()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ABaseSittableChair::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseSittableChair::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ABaseSittableChair::GetIsSeatOccupied()
{
	return bSeatIsOccupied;
}


bool ABaseSittableChair::SitActorOnChair(AActor* ActorToSitOnChair)
{
	if (bSeatIsOccupied || !IsValid(ActorToSitOnChair))
	{
		return false;
	}

	if (ACharacter* CharacterActor = Cast<ACharacter>(ActorToSitOnChair))
	{
		CharacterActor->GetCharacterMovement()->DisableMovement();
		CharacterActor->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	FAttachmentTransformRules AttachmentRules(
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::KeepWorld,
		false
	);

	ActorToSitOnChair->AttachToComponent(GetRootComponent(), AttachmentRules);

	OccupyingActor = ActorToSitOnChair;
	bSeatIsOccupied = true;

	return true;
}
