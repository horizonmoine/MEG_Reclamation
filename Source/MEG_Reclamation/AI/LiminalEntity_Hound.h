#pragma once

#include "CoreMinimal.h"
#include "AI/LiminalEntity.h"
#include "LiminalEntity_Hound.generated.h"

UCLASS()
class MEG_RECLAMATION_API ALiminalEntity_Hound : public ALiminalEntity
{
	GENERATED_BODY()

public:
	ALiminalEntity_Hound();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Combat")
	float ChargeSpeed = 750.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Combat")
	float NormalStalkSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Audio")
	float RMSTrackingThreshold = 0.03f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Audio")
	float MaxHearingRange = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Audio")
	float OcclusionFactor = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Behavior")
	float IntimidationRange = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Behavior")
	float DeAggroTime = 3.5f;

private:
	void UpdateAcousticTracking(float DeltaSeconds);

	bool bIsChargingByRMS = false;
	float CurrentDeAggroTimer = 0.0f;
	TWeakObjectPtr<AActor> LastAcousticTarget;
};
