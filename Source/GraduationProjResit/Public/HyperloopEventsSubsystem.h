// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HyperloopEventsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHyperloopDoorsOpening);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHyperloopDoorsClosing);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHyperloopDeparture);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHyperloopCinematicStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHyperloopCinematicComplete);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHyperloopAcceptingPassengers);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHyperloopPassengersBoarded);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBeginningCinematicFinished);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHUDFadeInRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHUDFadeOutRequested);

/**
 * engine managed world scoped singleton that manages all of the broadcastable and listenable events of the application for all classes :)
 * If refactoring this in the future, it may be a good idea to group broadcastable events by type as this project grows.
 */
UCLASS()
class GRADUATIONPROJRESIT_API UHyperloopEventsSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Hyperloop|Events")
	FOnHyperloopDoorsOpening OnDoorsOpening;

	UPROPERTY(BlueprintAssignable, Category = "Hyperloop|Events")
	FOnHyperloopDoorsClosing OnDoorsClosing;

	UPROPERTY(BlueprintAssignable, Category = "Hyperloop|Events")
	FOnHyperloopCinematicComplete OnCinematicComplete;

	UPROPERTY(BlueprintAssignable, Category = "Hyperloop|Events")
	FOnHyperloopCinematicComplete OnCinematicStart;

	UPROPERTY(BlueprintAssignable, Category = "Hyperloop|Events")
	FOnHyperloopDeparture OnDeparture;

	UPROPERTY(BlueprintAssignable, Category = "Hyperloop | Events")
	FOnHyperloopAcceptingPassengers OnAcceptingPassengers;

	UPROPERTY(BlueprintAssignable, Category = "Hyperloop | Events")
	FOnHyperloopPassengersBoarded OnPassengersBoarded;

	UPROPERTY(BlueprintAssignable, Category = "Hyperloop | Events")
	FOnHyperloopPassengersBoarded OnBeginningCinematicFinished;

	UPROPERTY(BlueprintAssignable, Category = "Hyperloop | Events")
	FOnHyperloopPassengersBoarded OnHUDFadeInRequested;

	UPROPERTY(BlueprintAssignable, Category = "Hyperloop | Events")
	FOnHyperloopPassengersBoarded OnHUDFadeOutRequested;

	UFUNCTION(BlueprintCallable, Category = "Hyperloop|Events")
	void BroadcastDoorsOpening();

	UFUNCTION(BlueprintCallable, Category = "Hyperloop|Events")
	void BroadcastDoorsClosing();

	UFUNCTION(BlueprintCallable, Category = "Hyperloop|Events")
	void BroadcastCinematicComplete();

	UFUNCTION(BlueprintCallable, Category = "Hyperloop|Events")
	void BroadcastCinematicStart();

	UFUNCTION(BlueprintCallable, Category = "Hyperloop|Events")
	void BroadcastDeparture();

	UFUNCTION(BlueprintCallable, Category = "Hyperloop | Events")
	void BroadcastAcceptingPassengers();

	UFUNCTION(BlueprintCallable, Category = "Hyperloop | Events")
	void BroadcastPassengersBoarded();

	UFUNCTION(BlueprintCallable, Category = "Hyperloop | Events")
	void BroadcastBeginningCinematicFinished();

	UFUNCTION(BlueprintCallable, Category = "Hyperloop | Events")
	void BroadcastHUDFadeInRequested();

	UFUNCTION(BlueprintCallable, Category = "Hyperloop | Events")
	void BroadcastHUDFadeOutRequested();
};