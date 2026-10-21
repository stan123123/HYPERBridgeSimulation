#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameAudioSubsystem.generated.h"

UCLASS()
class GRADUATIONPROJRESIT_API UGameAudioSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float HUDVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float SFXVolume = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio", meta = (WorldContext = "WorldContextObject"))
    static UGameAudioSubsystem* Get(const UObject* WorldContextObject);
};