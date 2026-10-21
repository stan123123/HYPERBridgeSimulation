// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Test123Class.generated.h"

UCLASS()
class GRADUATIONPROJRESIT_API ATest123Class : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATest123Class();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
