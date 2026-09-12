#include "GameModes/LiminalZoneRulesSubsystem.h"

#include "Engine/World.h"
#include "GameModes/LiminalGameState.h"
#include "Objects/LiminalSafeZoneVolume.h"

bool ULiminalZoneRulesSubsystem::IsMissionHostile() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const ALiminalGameState* GS = World->GetGameState<ALiminalGameState>();
	if (!IsValid(GS))
	{
		return false;
	}

	const EMissionPhase Phase = GS->GetMissionPhase();
	return Phase == EMissionPhase::Incursion || Phase == EMissionPhase::Collapsing;
}

bool ULiminalZoneRulesSubsystem::IsLocationInSafeZone(const FVector& Location) const
{
	return FindSafeZoneContaining(Location) != nullptr;
}

bool ULiminalZoneRulesSubsystem::IsSanityPressureActiveFor(const AActor* Actor) const
{
	if (!IsMissionHostile())
	{
		return false;
	}

	if (!IsValid(Actor))
	{
		return true;
	}

	return !IsLocationInSafeZone(Actor->GetActorLocation());
}

bool ULiminalZoneRulesSubsystem::IsEntitySpawnAllowedAt(const FVector& Location) const
{
	if (!IsMissionHostile())
	{
		return false;
	}

	const ALiminalSafeZoneVolume* Zone = FindSafeZoneContaining(Location);
	return !Zone || !Zone->BlocksEntitySpawns();
}

float ULiminalZoneRulesSubsystem::GetSafeZoneSanityRestoreRateFor(const AActor* Actor) const
{
	if (!IsValid(Actor))
	{
		return 0.0f;
	}

	const ALiminalSafeZoneVolume* Zone = FindSafeZoneContaining(Actor->GetActorLocation());
	return Zone ? Zone->GetSanityRestorePerSecond() : 0.0f;
}

bool ULiminalZoneRulesSubsystem::IsSanityPressureActive(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return true;
	}

	const UWorld* World = Actor->GetWorld();
	const ULiminalZoneRulesSubsystem* Rules = World ? World->GetSubsystem<ULiminalZoneRulesSubsystem>() : nullptr;
	return Rules ? Rules->IsSanityPressureActiveFor(Actor) : true;
}

float ULiminalZoneRulesSubsystem::GetSafeZoneSanityRestoreRate(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return 0.0f;
	}

	const UWorld* World = Actor->GetWorld();
	const ULiminalZoneRulesSubsystem* Rules = World ? World->GetSubsystem<ULiminalZoneRulesSubsystem>() : nullptr;
	return Rules ? Rules->GetSafeZoneSanityRestoreRateFor(Actor) : 0.0f;
}

void ULiminalZoneRulesSubsystem::RegisterSafeZone(ALiminalSafeZoneVolume* Zone)
{
	if (IsValid(Zone))
	{
		SafeZones.AddUnique(Zone);
	}
}

void ULiminalZoneRulesSubsystem::UnregisterSafeZone(ALiminalSafeZoneVolume* Zone)
{
	SafeZones.RemoveAll([Zone](const TWeakObjectPtr<ALiminalSafeZoneVolume>& Entry)
	{
		return !Entry.IsValid() || Entry.Get() == Zone;
	});
}

const ALiminalSafeZoneVolume* ULiminalZoneRulesSubsystem::FindSafeZoneContaining(const FVector& Location) const
{
	for (const TWeakObjectPtr<ALiminalSafeZoneVolume>& Entry : SafeZones)
	{
		const ALiminalSafeZoneVolume* Zone = Entry.Get();
		if (Zone && Zone->IsActive() && Zone->ContainsLocation(Location))
		{
			return Zone;
		}
	}
	return nullptr;
}
