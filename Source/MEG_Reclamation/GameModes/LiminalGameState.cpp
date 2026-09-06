#include "LiminalGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"

ALiminalGameState::ALiminalGameState()
{
	bIsBlackoutActive = false;
	CurrentStability = 100.0f;
}

void ALiminalGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALiminalGameState, bIsBlackoutActive);
	DOREPLIFETIME(ALiminalGameState, CurrentStability);
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

void ALiminalGameState::OnRep_CurrentStability()
{
	// Client-side visual/audio reactions when stability threshold crossed
	if (CurrentStability <= 20.0f)
	{
		// Collapse phase: rapid emergency lights / blackout pulses
	}
	else if (CurrentStability <= 60.0f)
	{
		// Destabilized phase: intermittent flickering
	}
}

EStabilityPhase ALiminalGameState::GetStabilityPhase() const
{
	if (CurrentStability > 60.0f)
	{
		return EStabilityPhase::Normal;
	}
	if (CurrentStability > 20.0f)
	{
		return EStabilityPhase::Destabilized;
	}
	return EStabilityPhase::Collapse;
}

void ALiminalGameState::SetStability(float NewStability)
{
	if (HasAuthority())
	{
		CurrentStability = FMath::Clamp(NewStability, 0.0f, 100.0f);
	}
}

void ALiminalGameState::DrainStability(float Amount)
{
	if (HasAuthority() && Amount > 0.0f)
	{
		SetStability(CurrentStability - Amount);
	}
}

