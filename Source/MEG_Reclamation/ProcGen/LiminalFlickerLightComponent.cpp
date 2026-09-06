#include "ProcGen/LiminalFlickerLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Math/UnrealMathUtility.h"

ULiminalFlickerLightComponent::ULiminalFlickerLightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	TargetLight = nullptr;
}

void ULiminalFlickerLightComponent::BeginPlay()
{
	Super::BeginPlay();

	TargetLight = GetOwner()->FindComponentByClass<UPointLightComponent>();
	if (TargetLight)
	{
		BaseIntensity = TargetLight->Intensity;
		RunningTime = FMath::RandRange(0.0f, 100.0f);
	}
}

void ULiminalFlickerLightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TargetLight)
	{
		RunningTime += DeltaTime * FlickerSpeed;
		float SineWave = FMath::Sin(RunningTime);
		float Noise = FMath::PerlinNoise1D(RunningTime * 2.0f) * NoiseIntensity;
		float Modulator = 1.0f + (SineWave * 0.2f) + Noise;
		TargetLight->SetIntensity(BaseIntensity * FMath::Clamp(Modulator, 0.1f, 1.5f));
	}
}
