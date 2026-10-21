#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/DataTable.h"
#include "../Public/Headers/HyperloopPodType.h"
#include "MyProjectSettings.generated.h"

///
/// This class is a lazy loader. Datatables can be assigned to it in the project settings and then it lazy loads them for classes
/// 

UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "My Project Settings"))
class GRADUATIONPROJRESIT_API UMyProjectSettings : public UDeveloperSettings
{
    GENERATED_BODY()
public:
    UMyProjectSettings();
    virtual FName GetCategoryName() const override { return FName("Game"); }

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> HyperloopPodTypeDataTable;

    static const UMyProjectSettings* Get() { return GetDefault<UMyProjectSettings>(); }

    UFUNCTION(BlueprintCallable, Category = "Data")
    static bool GetHyperloopPodTypeData(FName RowName, FHyperloopPodType& OutData);

private:
    UPROPERTY() mutable TObjectPtr<UDataTable> HyperloopPodTypeTableCache;

    UDataTable* LoadOrGetCached(const TSoftObjectPtr<UDataTable>& SoftTable, TObjectPtr<UDataTable>& Cache) const;
};