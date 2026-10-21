// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PlayerCharacterBase.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;

/// <summary>
/// Base player class. has simple controls and camera serup
/// </summary>

UCLASS()
class GRADUATIONPROJRESIT_API APlayerCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacterBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);
};