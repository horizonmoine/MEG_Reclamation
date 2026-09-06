#include "Tools/BaseTool.h"

#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

ABaseTool::ABaseTool()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = true;

	ToolMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ToolMesh"));
	SetRootComponent(ToolMesh);
}

void ABaseTool::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseTool, bIsActive);
	DOREPLIFETIME(ABaseTool, BatteryCharge);
}

void ABaseTool::OnRep_IsActive()
{
	SetActorTickEnabled(bIsActive);
}

void ABaseTool::OnRep_BatteryCharge()
{
}

bool ABaseTool::Activate()
{
	if (!CanActivate())
	{
		return false;
	}

	bIsActive = true;
	SetActorTickEnabled(true);
	return true;
}

void ABaseTool::Deactivate()
{
	if (!bIsActive)
	{
		return;
	}

	bIsActive = false;
	SetActorTickEnabled(false);
}

bool ABaseTool::IsActive() const
{
	return bIsActive;
}

bool ABaseTool::CanActivate() const
{
	return !bIsActive && BatteryCharge > 0.0f;
}

float ABaseTool::GetBatteryCharge() const
{
	return BatteryCharge;
}

float ABaseTool::GetMaxBatteryCharge() const
{
	return MaxBatteryCharge;
}

void ABaseTool::SetBatteryCharge(float NewCharge)
{
	BatteryCharge = FMath::Clamp(NewCharge, 0.0f, MaxBatteryCharge);

	if (bIsActive && BatteryCharge <= 0.0f)
	{
		Deactivate();
	}
}

void ABaseTool::ConsumeBattery(float DeltaSeconds)
{
	BatteryCharge = FMath::Max(BatteryCharge - ActiveBatteryDrainPerSecond * DeltaSeconds, 0.0f);

	if (BatteryCharge <= 0.0f)
	{
		Deactivate();
	}
}

void ABaseTool::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ConsumeBattery(DeltaSeconds);
}
