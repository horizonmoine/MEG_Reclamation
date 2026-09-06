#include "LiminalGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"

ALiminalGameState::ALiminalGameState()
{
	bIsBlackoutActive = false;
}

void ALiminalGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALiminalGameState, bIsBlackoutActive);
}

void ALiminalGameState::OnRep_BlackoutActive()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	float TargetIntensity = bIsBlackoutActive ? 0.0f : 1400.0f;

	for (TActorIterator<APointLight> It(World); It; ++It)
	{
		if (APointLight* Lamp = *It)
		{
			if (UPointLightComponent* Comp = Cast<UPointLightComponent>(Lamp->GetLightComponent()))
			{
				Comp->SetIntensity(TargetIntensity);
			}
		}
	}
}
