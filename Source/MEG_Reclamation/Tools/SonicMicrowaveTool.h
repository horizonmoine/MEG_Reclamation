#pragma once

#include "CoreMinimal.h"
#include "Tools/BaseTool.h"
#include "SonicMicrowaveTool.generated.h"

UCLASS(Blueprintable)
class MEG_RECLAMATION_API ASonicMicrowaveTool : public ABaseTool
{
	GENERATED_BODY()

public:
	ASonicMicrowaveTool();

	virtual bool Activate() override;
	virtual bool CanActivate() const override;

protected:
	UFUNCTION()
	void EndActiveWindow();

	UFUNCTION()
	void ClearOverheat();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Microwave", meta = (ClampMin = "0.0"))
	float Range = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Microwave", meta = (ClampMin = "0.0"))
	float KnockbackStrength = 80000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Microwave", meta = (ClampMin = "0.01"))
	float ActiveWindowSeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Microwave", meta = (ClampMin = "0.1"))
	float OverheatCooldownSeconds = 2.5f;

private:
	void FirePulse();

	FTimerHandle ActiveWindowTimerHandle;
	FTimerHandle OverheatTimerHandle;

	bool bIsOverheated = false;
};
