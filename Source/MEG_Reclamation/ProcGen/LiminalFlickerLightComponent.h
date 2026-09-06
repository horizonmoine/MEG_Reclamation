#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LiminalFlickerLightComponent.generated.h"

class UPointLightComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MEG_RECLAMATION_API ULiminalFlickerLightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULiminalFlickerLightComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	UPointLightComponent* TargetLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker")
	float BaseIntensity = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker")
	float FlickerSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker")
	float NoiseIntensity = 0.5f;

private:
	float RunningTime = 0.0f;
};
