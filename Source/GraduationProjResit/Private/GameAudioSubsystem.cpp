#include "GameAudioSubsystem.h"

UGameAudioSubsystem* UGameAudioSubsystem::Get(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return nullptr;
    }

    if (UWorld* World = WorldContextObject->GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UGameAudioSubsystem>();
        }
    }

    return nullptr;
}