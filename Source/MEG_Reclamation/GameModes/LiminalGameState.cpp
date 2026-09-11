#include "LiminalGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogMEGMission, Log, All);

ALiminalGameState::ALiminalGameState()
	: bIsBlackoutActive(false)
	, CurrentStability(100.0f)
	, MissionPhase(EMissionPhase::Hub)
	, CollapseStartServerTime(0.0f)
	, CollapseDurationSeconds(480.0f)
	, bCollapseTimerRunning(false)
	, MissionSeed(0)
	, TeamBankCredits(0)
{
}

void ALiminalGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALiminalGameState, bIsBlackoutActive);
	DOREPLIFETIME(ALiminalGameState, CurrentStability);
	DOREPLIFETIME(ALiminalGameState, MissionPhase);
	DOREPLIFETIME(ALiminalGameState, CollapseStartServerTime);
	DOREPLIFETIME(ALiminalGameState, CollapseDurationSeconds);
	DOREPLIFETIME(ALiminalGameState, bCollapseTimerRunning);
	DOREPLIFETIME(ALiminalGameState, MissionSeed);
	DOREPLIFETIME(ALiminalGameState, TeamBankCredits);
}

// ---------------------------------------------------------------------------
// Blackout (existant)
// ---------------------------------------------------------------------------

void ALiminalGameState::OnRep_BlackoutActive()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float TargetIntensity = bIsBlackoutActive ? 0.0f : 1400.0f;

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

// ---------------------------------------------------------------------------
// Stabilite
// ---------------------------------------------------------------------------

void ALiminalGameState::OnRep_CurrentStability()
{
	// Reactions visuelles/audio client au franchissement de seuil : branchees par les subsystems locaux.
}

EStabilityPhase ALiminalGameState::GetStabilityPhase() const
{
	// Quand le timer tourne, la stabilite est le ratio de temps restant (source unique de verite).
	const float Ratio = bCollapseTimerRunning
		? (1.0f - GetCollapseProgress()) * 100.0f
		: CurrentStability;

	if (Ratio > 60.0f)
	{
		return EStabilityPhase::Normal;
	}
	if (Ratio > 20.0f)
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

// ---------------------------------------------------------------------------
// Phase de mission
// ---------------------------------------------------------------------------

bool ALiminalGameState::AuthSetMissionPhase(EMissionPhase NewPhase)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!IsPhaseTransitionValid(MissionPhase, NewPhase))
	{
		UE_LOG(LogMEGMission, Warning, TEXT("[%s] Transition de phase refusee : %d -> %d"),
			*GetName(), static_cast<int32>(MissionPhase), static_cast<int32>(NewPhase));
		return false;
	}

	const EMissionPhase OldPhase = MissionPhase;
	MissionPhase = NewPhase;
	OnRep_MissionPhase(OldPhase);
	return true;
}

bool ALiminalGameState::IsPhaseTransitionValid(EMissionPhase From, EMissionPhase To)
{
	if (From == To)
	{
		return false;
	}

	switch (From)
	{
	case EMissionPhase::Hub:
		return To == EMissionPhase::Airlock || To == EMissionPhase::Incursion;
	case EMissionPhase::Airlock:
		return To == EMissionPhase::Incursion || To == EMissionPhase::Hub;
	case EMissionPhase::Incursion:
		return To == EMissionPhase::Collapsing || To == EMissionPhase::Extracted || To == EMissionPhase::Failed;
	case EMissionPhase::Collapsing:
		return To == EMissionPhase::Extracted || To == EMissionPhase::Failed;
	case EMissionPhase::Extracted:
	case EMissionPhase::Failed:
		return To == EMissionPhase::Hub;
	default:
		return false;
	}
}

void ALiminalGameState::OnRep_MissionPhase(EMissionPhase OldPhase)
{
	OnMissionPhaseChanged.Broadcast(OldPhase, MissionPhase);
}

// ---------------------------------------------------------------------------
// Timer d'effondrement
// ---------------------------------------------------------------------------

void ALiminalGameState::AuthStartCollapseTimer(float DurationSeconds)
{
	if (!HasAuthority())
	{
		return;
	}

	CollapseDurationSeconds = FMath::Max(1.0f, DurationSeconds);
	CollapseStartServerTime = static_cast<float>(GetServerWorldTimeSeconds());
	bCollapseTimerRunning = true;
}

void ALiminalGameState::AuthStopCollapseTimer()
{
	if (!HasAuthority())
	{
		return;
	}

	bCollapseTimerRunning = false;
}

float ALiminalGameState::GetCollapseTimeRemaining() const
{
	if (!bCollapseTimerRunning)
	{
		return CollapseDurationSeconds;
	}

	const float Elapsed = static_cast<float>(GetServerWorldTimeSeconds()) - CollapseStartServerTime;
	return FMath::Clamp(CollapseDurationSeconds - Elapsed, 0.0f, CollapseDurationSeconds);
}

float ALiminalGameState::GetCollapseProgress() const
{
	if (CollapseDurationSeconds <= 0.0f)
	{
		return 1.0f;
	}

	return 1.0f - (GetCollapseTimeRemaining() / CollapseDurationSeconds);
}

// ---------------------------------------------------------------------------
// Seed & economie
// ---------------------------------------------------------------------------

void ALiminalGameState::AuthSetMissionSeed(int32 NewSeed)
{
	if (!HasAuthority() || MissionSeed == NewSeed)
	{
		return;
	}

	MissionSeed = NewSeed;
	OnRep_MissionSeed();
}

void ALiminalGameState::OnRep_MissionSeed()
{
	// Les clients late-join regenerent la geometrie ISM locale a partir de cette seed.
}

void ALiminalGameState::AuthAddTeamBankCredits(int32 Delta)
{
	if (!HasAuthority() || Delta == 0)
	{
		return;
	}

	const int32 NewValue = FMath::Max(0, TeamBankCredits + Delta);
	if (NewValue == TeamBankCredits)
	{
		return;
	}

	TeamBankCredits = NewValue;
	OnRep_TeamBankCredits();
}

void ALiminalGameState::OnRep_TeamBankCredits()
{
	OnTeamBankCreditsChanged.Broadcast(TeamBankCredits);
}

