#include "Objects/LiminalSafeZoneVolume.h"

#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameModes/LiminalZoneRulesSubsystem.h"

ALiminalSafeZoneVolume::ALiminalSafeZoneVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	Bounds = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounds"));
	SetRootComponent(Bounds);
	Bounds->SetBoxExtent(FVector(1000.0f, 1000.0f, 300.0f));
	Bounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Bounds->SetGenerateOverlapEvents(false);
	Bounds->SetHiddenInGame(true);
}

void ALiminalSafeZoneVolume::PostActorCreated()
{
	Super::PostActorCreated();

	if (UWorld* World = GetWorld())
	{
		if (World->IsGameWorld())
		{
			if (ULiminalZoneRulesSubsystem* Rules = World->GetSubsystem<ULiminalZoneRulesSubsystem>())
			{
				Rules->RegisterSafeZone(this);
			}
		}
	}
}

void ALiminalSafeZoneVolume::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (ULiminalZoneRulesSubsystem* Rules = World->GetSubsystem<ULiminalZoneRulesSubsystem>())
		{
			Rules->RegisterSafeZone(this);
		}
	}
}

void ALiminalSafeZoneVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (ULiminalZoneRulesSubsystem* Rules = World->GetSubsystem<ULiminalZoneRulesSubsystem>())
		{
			Rules->UnregisterSafeZone(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

bool ALiminalSafeZoneVolume::ContainsLocation(const FVector& Location) const
{
	if (!bIsActive || !IsValid(Bounds))
	{
		return false;
	}

	return Bounds->CalcBounds(Bounds->GetComponentTransform()).GetBox().IsInsideOrOn(Location);
}

void ALiminalSafeZoneVolume::SetActive(bool bActive)
{
	bIsActive = bActive;
}

void ALiminalSafeZoneVolume::SetBoxExtent(const FVector& NewExtent)
{
	if (IsValid(Bounds))
	{
		Bounds->SetBoxExtent(NewExtent, true);
	}
}
