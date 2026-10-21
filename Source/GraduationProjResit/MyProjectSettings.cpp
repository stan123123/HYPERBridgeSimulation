#include "MyProjectSettings.h"

UMyProjectSettings::UMyProjectSettings() {}

UDataTable* UMyProjectSettings::LoadOrGetCached(const TSoftObjectPtr<UDataTable>& SoftTable, TObjectPtr<UDataTable>& Cache) const
{
    if (Cache) return Cache;
    if (SoftTable.IsNull()) return nullptr;
    Cache = SoftTable.LoadSynchronous();
    return Cache;
}

bool UMyProjectSettings::GetHyperloopPodTypeData(FName RowName, FHyperloopPodType& OutData)
{
    const UMyProjectSettings* Settings = Get();
    if (UDataTable* Table = Settings->LoadOrGetCached(Settings->HyperloopPodTypeDataTable, Settings->HyperloopPodTypeTableCache))
    {
        if (FHyperloopPodType* Row = Table->FindRow<FHyperloopPodType>(RowName, TEXT("Hyperloop Pod Type Context")))
        {
            OutData = *Row;
            return true;
        }
    }
    return false;
}