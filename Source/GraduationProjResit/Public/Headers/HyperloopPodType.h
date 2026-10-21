#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "Sound/SoundBase.h"
#include "HyperloopPodType.generated.h"

/**
 * Data table row structure for polymorphic pod type
 */
	USTRUCT(BlueprintType)
	struct FHyperloopPodType : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pod Configuration")
	TSoftObjectPtr<USkeletalMesh> PodSkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pod Animations")
	TSoftObjectPtr<UAnimMontage> OpenAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pod Animations")
	TSoftObjectPtr<UAnimMontage> CloseAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pod Audio")
	TSoftObjectPtr<USoundBase> DoorOpenSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pod Audio")
	TSoftObjectPtr<USoundBase> DoorCloseSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pod Physics", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float GForceAcceleration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pod Physics", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float InterpSpeedToAcceleration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pod Physics", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MaxSpeed = 1.0f;
};