// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Headers/HyperloopPodType.h"
#include "BaseHyperloopPod.generated.h"

/// <summary>
/// This class is responsible for the pod's behavior when traveling etc. Important note, the pod doesnt have an anim instance or animbp, as can be seen in the animation handlers,
/// It handles animations via applying animmontages to the skeleton directly. Keep this in mind if planning to add something like a ABP or something alike in the future.
/// 
/// This class is also polymorphic. the pod type can be changed by setting the public member "DataRowName"
/// </summary>

class UBoxComponent;
class USkeletalMeshComponent;
class APlayerCharacterBase;
class ABaseSittableChair;

UENUM(BlueprintType)
enum class EHyperloopPodState : uint8
{
	Traveling,
	DoorsOpening,
	BoardingCooldown,
	ReadyForDeparture,
	DoorsClosing
};

UCLASS()
class GRADUATIONPROJRESIT_API ABaseHyperloopPod : public AActor
{
	GENERATED_BODY()
public:
	ABaseHyperloopPod();

	UFUNCTION(BlueprintCallable, Category = "Hyperloop|Destinations")
	void PushDestination(AActor* Destination);

	UFUNCTION(BlueprintCallable, Category = "Hyperloop|Destinations")
	AActor* GetCurrentDestination() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	FName DataRowName;

	UFUNCTION()
	TArray<ABaseSittableChair*> GetAllSittableChairs();

protected:

	// these 2 members are only for the inherited classes to read. they dont actually affect the physics etc.
	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	float TotalDistanceToDestination = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Navigation")
	float DistanceRemainingToDestination = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class USceneComponent* DoorSoundsSource;

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	void SetState(EHyperloopPodState NewState);
	void TickTraveling(float DeltaTime);

	virtual void BeginDoorsOpening();
	virtual void BeginDoorsClosing();

	UFUNCTION(BlueprintCallable, Category = "Hyperloop|Doors")
	void NotifyDoorsOpened();
	UFUNCTION(BlueprintCallable, Category = "Hyperloop|Doors")
	void NotifyDoorsClosed();

	UFUNCTION()
	void OnBoardingCooldownExpired();
	UFUNCTION()
	void OnEntranceHitboxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	virtual void Tick(float DeltaTime) override;

private:

	FTimerHandle DoorAnimTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Hyperloop|Config")
	FHyperloopPodType PodData;

	UPROPERTY(EditInstanceOnly, Category = "Hyperloop|Navigation")
	TArray<TLazyObjectPtr<AActor>> DestinationStack;

	EHyperloopPodState CurrentState = EHyperloopPodState::Traveling;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hyperloop|Collision", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> HazardHitbox;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hyperloop|Collision", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> EntranceHitbox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hyperloop|Visuals", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> PodMesh;

	FTimerHandle BoardingCooldownTimerHandle;

	static constexpr float BoardingCooldownDuration = 5.0f;
	static constexpr float ArrivalMarginCm = 1.0f;
	static constexpr float StoppingDistanceMarginCm = 0.0f;
	static constexpr float GForceToCmPerSecSq = 980.665f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hyperloop|Physics", meta = (AllowPrivateAccess = "true"))
	float CurrentSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hyperloop|Physics", meta = (AllowPrivateAccess = "true"))
	float CurrentGForce = 0.0f;

	static constexpr float InterpConvergence99PctFactor = 4.6f;
};