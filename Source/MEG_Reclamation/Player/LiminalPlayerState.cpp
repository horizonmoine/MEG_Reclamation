#include "Player/LiminalPlayerState.h"

#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogMEGPlayerState, Log, All);

ALiminalPlayerState::ALiminalPlayerState()
	: Sanity(100.0f)
	, Health(100.0f)
	, Status(EScavengerStatus::Alive)
	, CarriedCredits(0)
	, bIsInfectedPartygoer(false)
	, bIsReady(false)
	, MaxSanity(100.0f)
	, MaxHealth(100.0f)
{
	bReplicates = true;
}

void ALiminalPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALiminalPlayerState, Sanity);
	DOREPLIFETIME(ALiminalPlayerState, Health);
	DOREPLIFETIME(ALiminalPlayerState, Status);
	DOREPLIFETIME(ALiminalPlayerState, CarriedCredits);
	DOREPLIFETIME(ALiminalPlayerState, bIsInfectedPartygoer);
	DOREPLIFETIME(ALiminalPlayerState, bIsReady);
}

void ALiminalPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	ALiminalPlayerState* Target = Cast<ALiminalPlayerState>(PlayerState);
	if (!IsValid(Target))
	{
		return;
	}

	Target->Sanity = Sanity;
	Target->Health = Health;
	Target->Status = Status;
	Target->CarriedCredits = CarriedCredits;
	Target->bIsInfectedPartygoer = bIsInfectedPartygoer;
	Target->bIsReady = bIsReady;
}

void ALiminalPlayerState::OverrideWith(APlayerState* PlayerState)
{
	Super::OverrideWith(PlayerState);

	const ALiminalPlayerState* Source = Cast<ALiminalPlayerState>(PlayerState);
	if (!IsValid(Source))
	{
		return;
	}

	Sanity = Source->Sanity;
	Health = Source->Health;
	Status = Source->Status;
	CarriedCredits = Source->CarriedCredits;
	bIsInfectedPartygoer = Source->bIsInfectedPartygoer;
	bIsReady = Source->bIsReady;
}

// ---------------------------------------------------------------------------
// Mutations serveur
// ---------------------------------------------------------------------------

void ALiminalPlayerState::AuthApplySanityDelta(float Delta)
{
	if (!HasAuthority() || FMath::IsNearlyZero(Delta))
	{
		return;
	}

	SetSanityInternal(Sanity + Delta);

	if (Sanity <= 0.0f)
	{
		HandleIncapacitation();
	}
}

void ALiminalPlayerState::AuthApplyHealthDelta(float Delta)
{
	if (!HasAuthority() || FMath::IsNearlyZero(Delta))
	{
		return;
	}

	SetHealthInternal(Health + Delta);

	if (Health <= 0.0f)
	{
		HandleIncapacitation();
	}
}

bool ALiminalPlayerState::AuthSetStatus(EScavengerStatus NewStatus)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!IsStatusTransitionValid(Status, NewStatus))
	{
		UE_LOG(LogMEGPlayerState, Warning, TEXT("[%s] Transition de statut refusee : %d -> %d"),
			*GetName(), static_cast<int32>(Status), static_cast<int32>(NewStatus));
		return false;
	}

	const EScavengerStatus OldStatus = Status;
	Status = NewStatus;
	OnRep_Status(OldStatus);
	return true;
}

void ALiminalPlayerState::AuthAddCarriedCredits(int32 Delta)
{
	if (!HasAuthority() || Delta == 0)
	{
		return;
	}

	AuthSetCarriedCredits(CarriedCredits + Delta);
}

void ALiminalPlayerState::AuthSetCarriedCredits(int32 NewCredits)
{
	if (!HasAuthority())
	{
		return;
	}

	const int32 Clamped = FMath::Max(0, NewCredits);
	if (Clamped == CarriedCredits)
	{
		return;
	}

	CarriedCredits = Clamped;
	OnRep_CarriedCredits();
}

void ALiminalPlayerState::AuthSetInfectedPartygoer(bool bInfected)
{
	if (!HasAuthority() || bIsInfectedPartygoer == bInfected)
	{
		return;
	}

	bIsInfectedPartygoer = bInfected;
	OnRep_InfectedPartygoer();
}

void ALiminalPlayerState::AuthSetReady(bool bReady)
{
	if (!HasAuthority() || bIsReady == bReady)
	{
		return;
	}

	bIsReady = bReady;
	OnRep_Ready();
}

void ALiminalPlayerState::AuthRevive(float HealthPercent, float SanityPercent)
{
	if (!HasAuthority() || Status != EScavengerStatus::Downed)
	{
		return;
	}

	SetHealthInternal(MaxHealth * FMath::Clamp(HealthPercent, 0.05f, 1.0f));
	SetSanityInternal(MaxSanity * FMath::Clamp(SanityPercent, 0.05f, 1.0f));
	AuthSetStatus(EScavengerStatus::Alive);
}

void ALiminalPlayerState::AuthResetForNewRun()
{
	if (!HasAuthority())
	{
		return;
	}

	SetHealthInternal(MaxHealth);
	SetSanityInternal(MaxSanity);

	if (Status != EScavengerStatus::Alive)
	{
		const EScavengerStatus OldStatus = Status;
		Status = EScavengerStatus::Alive;
		OnRep_Status(OldStatus);
	}

	AuthSetCarriedCredits(0);
	AuthSetInfectedPartygoer(false);
	AuthSetReady(false);
}

// ---------------------------------------------------------------------------
// Lecture
// ---------------------------------------------------------------------------

float ALiminalPlayerState::GetSanityPercent() const
{
	return MaxSanity > 0.0f ? FMath::Clamp(Sanity / MaxSanity, 0.0f, 1.0f) : 0.0f;
}

float ALiminalPlayerState::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f) : 0.0f;
}

bool ALiminalPlayerState::IsStatusTransitionValid(EScavengerStatus From, EScavengerStatus To)
{
	if (From == To)
	{
		return false;
	}

	switch (From)
	{
	case EScavengerStatus::Alive:
		return To == EScavengerStatus::Downed || To == EScavengerStatus::Dead || To == EScavengerStatus::Extracted;
	case EScavengerStatus::Downed:
		return To == EScavengerStatus::Alive || To == EScavengerStatus::Dead;
	case EScavengerStatus::Dead:
		return To == EScavengerStatus::Spectating;
	case EScavengerStatus::Extracted:
		return To == EScavengerStatus::Spectating;
	case EScavengerStatus::Spectating:
		return To == EScavengerStatus::Alive;
	default:
		return false;
	}
}

// ---------------------------------------------------------------------------
// OnRep (appeles automatiquement sur clients, manuellement sur serveur)
// ---------------------------------------------------------------------------

void ALiminalPlayerState::OnRep_Sanity(float OldSanity)
{
	OnSanityChanged.Broadcast(OldSanity, Sanity);
}

void ALiminalPlayerState::OnRep_Health(float OldHealth)
{
	OnHealthChanged.Broadcast(OldHealth, Health);
}

void ALiminalPlayerState::OnRep_Status(EScavengerStatus OldStatus)
{
	OnStatusChanged.Broadcast(OldStatus, Status);
}

void ALiminalPlayerState::OnRep_CarriedCredits()
{
	OnCarriedCreditsChanged.Broadcast(CarriedCredits);
}

void ALiminalPlayerState::OnRep_InfectedPartygoer()
{
	// Feedback local (post-process, HUD) branche par les composants owner-only.
}

void ALiminalPlayerState::OnRep_Ready()
{
	// Le lobby lit IsReady() via le GameState pour afficher l'etat de l'equipe.
}

// ---------------------------------------------------------------------------
// Interne
// ---------------------------------------------------------------------------

void ALiminalPlayerState::HandleIncapacitation()
{
	if (Status == EScavengerStatus::Alive)
	{
		AuthSetStatus(EScavengerStatus::Downed);
	}
}

void ALiminalPlayerState::SetSanityInternal(float NewSanity)
{
	const float OldSanity = Sanity;
	Sanity = FMath::Clamp(NewSanity, 0.0f, MaxSanity);
	if (!FMath::IsNearlyEqual(OldSanity, Sanity))
	{
		OnRep_Sanity(OldSanity);
	}
}

void ALiminalPlayerState::SetHealthInternal(float NewHealth)
{
	const float OldHealth = Health;
	Health = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
	if (!FMath::IsNearlyEqual(OldHealth, Health))
	{
		OnRep_Health(OldHealth);
	}
}
