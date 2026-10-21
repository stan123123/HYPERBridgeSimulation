// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Templates/SubclassOf.h"
#include "NPC_Character.generated.h"

class ANPC_Waypoint;
class ABaseSittableChair;
class ABaseHyperloopPod;

UCLASS()
class GRADUATIONPROJRESIT_API ANPC_Character : public ACharacter
{
	GENERATED_BODY()

public:
	ANPC_Character();

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	UPROPERTY(EditInstanceOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* NPCStaticMesh;

	UPROPERTY(EditInstanceOnly, Category = "NPC navigation", meta = (AllowPrivateAccess = "true"))
	TArray<TLazyObjectPtr<ANPC_Waypoint>> NavigationWaypointStack;

	UPROPERTY(EditInstanceOnly, Category = "NPC navigation", meta = (AllowPrivateAccess = "true"))
	float NavigationStartDelaySeconds = 0.1f;

	UPROPERTY(EditInstanceOnly, Category = "NPC navigation", meta = (AllowPrivateAccess = "true"))
	ABaseHyperloopPod* TargetHyperloopPod  = nullptr;

	UPROPERTY()
	FTimerHandle NavigationStartDelayTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bIsNavigating = false;

	// private member to track sitting state for animations
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bIsSitting = false;

	const float MoveSpeed = 180.0f;
	const float RotationChangeInterpSpeed = 5.0f;
	const float RotationChangeSpeed = 180.0f;
	const float TargetDestinationFacingThreshold = 60.0f;
	const float WaypointReachedRadius = 15.0f;

	float InterpedRotationalModifier = 0.0f;
	const float MinimumInterpedRotationalModifier = 0.2f;

public:

	UPROPERTY()
	ANPC_Waypoint* CurrentWayPoint = nullptr;

private:

	UFUNCTION()
	void HandleAcceptingPassengers();

	UFUNCTION()
	void StartNavigation();

	UFUNCTION()
	ANPC_Waypoint* GetNextWayPoint();

};
