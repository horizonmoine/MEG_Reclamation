#pragma once

#include "CoreMinimal.h"
#include "Tools/BaseTool.h"
#include "FlashStrobeTool.generated.h"

class UPointLightComponent;

UCLASS(Blueprintable)
class MEG_RECLAMATION_API AFlashStrobeTool : public ABaseTool
{
	GENERATED_BODY()

public:
	AFlashStrobeTool();

	virtual bool Activate() override;
	virtual void Deactivate() override;
	virtual void OnRep_IsActive() override;

protected:
	void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Strobe", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPointLightComponent> StrobeLight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Strobe", meta = (ClampMin = "0.0"))
	float MaxIntensity = 50000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Strobe", meta = (ClampMin = "0.1"))
	float StrobeFrequencyHz = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Strobe", meta = (ClampMin = "100.0"))
	float StrobeRange = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Strobe", meta = (ClampMin = "0.5"))
	float StunDuration = 2.5f;

private:
	float StrobePhaseSeconds = 0.0f;
};
