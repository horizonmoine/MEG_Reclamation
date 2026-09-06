#include "Tools/WalkieTalkieTool.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

AWalkieTalkieTool::AWalkieTalkieTool()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MaxBatteryCharge = 100.0f;
	BatteryCharge = 100.0f;
	ActiveBatteryDrainPerSecond = 2.0f;
	CurrentChannel = 1;
	MaxEffectiveRangeMeters = 120.0f;
	AcousticNoiseRadiusOnTransmit = 600.0f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> WalkieFinder(
		TEXT("/Game/Meshes/Tools/SM_WalkieTalkie.SM_WalkieTalkie"));
	if (WalkieFinder.Succeeded() && ToolMesh)
	{
		ToolMesh->SetStaticMesh(WalkieFinder.Object);
		ToolMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	}
}

void AWalkieTalkieTool::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWalkieTalkieTool, CurrentChannel);
}

bool AWalkieTalkieTool::Activate()
{
	if (!CanActivate())
	{
		return false;
	}

	const bool bStarted = Super::Activate();
	if (bStarted)
	{
		if (HasAuthority())
		{
			// Le clic PTT emet un bruit leger qui peut alerter une entite tres proche
			MakeNoise(0.3f, GetInstigator(), GetActorLocation(), AcousticNoiseRadiusOnTransmit);
		}
		OnTransmitStateChanged.Broadcast(true, CurrentChannel);
	}
	return bStarted;
}

void AWalkieTalkieTool::Deactivate()
{
	if (bIsActive)
	{
		Super::Deactivate();
		OnTransmitStateChanged.Broadcast(false, CurrentChannel);
	}
}

void AWalkieTalkieTool::SetChannel(int32 NewChannel)
{
	const int32 Clamped = FMath::Clamp(NewChannel, 1, 8);
	if (CurrentChannel != Clamped)
	{
		CurrentChannel = Clamped;
		OnChannelChanged.Broadcast(CurrentChannel, this);
	}
}

void AWalkieTalkieTool::NextChannel()
{
	int32 Next = CurrentChannel + 1;
	if (Next > 8)
	{
		Next = 1;
	}
	SetChannel(Next);
}

void AWalkieTalkieTool::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsActive && HasAuthority())
	{
		// Bruit continu pendant la transmission
		MakeNoise(0.2f, GetInstigator(), GetActorLocation(), AcousticNoiseRadiusOnTransmit * 0.75f);
	}
}

float AWalkieTalkieTool::CalculateSignalClarity(const FVector& TransmitterLocation) const
{
	const float DistCm = FVector::Distance(GetActorLocation(), TransmitterLocation);
	const float DistMeters = DistCm / 100.0f;

	if (DistMeters >= MaxEffectiveRangeMeters)
	{
		return 0.0f;
	}

	float Clarity = 1.0f - (DistMeters / MaxEffectiveRangeMeters);

	// Attenuation par penetration des obstacles (murs / dalles)
	if (UWorld* World = GetWorld())
	{
		TArray<FHitResult> Hits;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		if (GetInstigator())
		{
			QueryParams.AddIgnoredActor(GetInstigator());
		}

		const bool bHit = World->LineTraceMultiByChannel(
			Hits,
			TransmitterLocation,
			GetActorLocation(),
			ECC_Visibility,
			QueryParams
		);

		if (bHit)
		{
			// Chaque mur traverse degrade la puissance radio de 15%
			const float ObstacleLoss = Hits.Num() * 0.15f;
			Clarity = FMath::Clamp(Clarity - ObstacleLoss, 0.0f, 1.0f);
		}
	}

	return FMath::Clamp(Clarity, 0.0f, 1.0f);
}

void AWalkieTalkieTool::OnRep_CurrentChannel()
{
	OnChannelChanged.Broadcast(CurrentChannel, this);
}

void AWalkieTalkieTool::OnRep_IsActive()
{
	Super::OnRep_IsActive();
	OnTransmitStateChanged.Broadcast(bIsActive, CurrentChannel);
}
