// Fill out your copyright notice in the Description page of Project Settings.

#include "HyperloopEventsSubsystem.h"

void UHyperloopEventsSubsystem::BroadcastDoorsOpening()
{
	OnDoorsOpening.Broadcast();
}

void UHyperloopEventsSubsystem::BroadcastDoorsClosing()
{
	OnDoorsClosing.Broadcast();
}

void UHyperloopEventsSubsystem::BroadcastCinematicComplete()
{
	OnCinematicComplete.Broadcast();
}

void UHyperloopEventsSubsystem::BroadcastCinematicStart()
{
	OnCinematicStart.Broadcast();
}

void UHyperloopEventsSubsystem::BroadcastDeparture()
{
	OnDeparture.Broadcast();
}

void UHyperloopEventsSubsystem::BroadcastAcceptingPassengers()
{
	OnAcceptingPassengers.Broadcast();
}

void UHyperloopEventsSubsystem::BroadcastPassengersBoarded()
{
	OnPassengersBoarded.Broadcast();
}

void UHyperloopEventsSubsystem::BroadcastBeginningCinematicFinished()
{
	OnBeginningCinematicFinished.Broadcast();
}

void UHyperloopEventsSubsystem::BroadcastHUDFadeInRequested()
{
	OnHUDFadeInRequested.Broadcast();
}

void UHyperloopEventsSubsystem::BroadcastHUDFadeOutRequested()
{
	OnHUDFadeOutRequested.Broadcast();
}