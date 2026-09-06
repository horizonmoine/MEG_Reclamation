#include "Tools/LidarScannerTool.h"
#include "AI/LiminalEntity_Duller.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "CollisionQueryParams.h"
#include "UObject/ConstructorHelpers.h"

ALidarScannerTool::ALidarScannerTool()
{
	MaxBatteryCharge = 100.0f;
	BatteryCharge = 100.0f;
	ActiveBatteryDrainPerSecond = 0.0f; // Uses discrete pulses

	static ConstructorHelpers::FObjectFinder<UStaticMesh> LidarFinder(
		TEXT("/Game/Meshes/Tools/SM_Lidar_Scanner.SM_Lidar_Scanner"));
	if (LidarFinder.Succeeded() && ToolMesh)
	{
		ToolMesh->SetStaticMesh(LidarFinder.Object);
		ToolMesh->SetRelativeScale3D(FVector(1.0f));
	}
}

bool ALidarScannerTool::CanActivate() const
{
	return Super::CanActivate() && CooldownTimer <= 0.0f && BatteryCharge >= BatteryCostPerPulse;
}

bool ALidarScannerTool::Activate()
{
	if (!CanActivate())
	{
		return false;
	}

	SetBatteryCharge(FMath::Max(0.0f, BatteryCharge - BatteryCostPerPulse));
	CooldownTimer = ScanCooldownSeconds;

	ExecuteLidarSweep();
	return true;
}

void ALidarScannerTool::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CooldownTimer > 0.0f)
	{
		CooldownTimer -= DeltaSeconds;
	}

	// Slow passive recharge
	if (BatteryCharge < MaxBatteryCharge)
	{
		SetBatteryCharge(FMath::Min(MaxBatteryCharge, BatteryCharge + 2.5f * DeltaSeconds));
	}
}

void ALidarScannerTool::ExecuteLidarSweep()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Origin = GetActorLocation();
	const FVector Forward = GetActorForwardVector();

	int32 PointsDetected = 0;
	int32 EntitiesRevealed = 0;

	// Perform raycast sweep for geometry
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	if (AActor* OwnerActor = GetOwner())
	{
		QueryParams.AddIgnoredActor(OwnerActor);
	}

	constexpr int32 NumRays = 16;
	const float HalfAngleRad = FMath::DegreesToRadians(ScanConeAngle * 0.5f);

	for (int32 i = 0; i < NumRays; ++i)
	{
		const float PitchOffset = FMath::FRandRange(-HalfAngleRad, HalfAngleRad);
		const float YawOffset = FMath::FRandRange(-HalfAngleRad, HalfAngleRad);

		const FVector RayDir = Forward.RotateAngleAxis(FMath::RadiansToDegrees(YawOffset), FVector::UpVector)
			.RotateAngleAxis(FMath::RadiansToDegrees(PitchOffset), GetActorRightVector());

		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, Origin, Origin + RayDir * ScanRange, ECC_WorldStatic, QueryParams))
		{
			PointsDetected++;
		}
	}

	// Reveal cloaked Dullers within the scan cone
	for (TActorIterator<ALiminalEntity_Duller> It(World); It; ++It)
	{
		ALiminalEntity_Duller* Duller = *It;
		if (Duller)
		{
			const FVector ToDuller = Duller->GetActorLocation() - Origin;
			const float Dist = ToDuller.Size();

			if (Dist <= ScanRange)
			{
				const float Dot = FVector::DotProduct(Forward, ToDuller.GetSafeNormal());
				const float MinDot = FMath::Cos(HalfAngleRad);

				if (Dot >= MinDot)
				{
					Duller->RevealFromScanner(DullerRevealDuration);
					EntitiesRevealed++;
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[LIDAR] Pulse complete: %d points, %d entities revealed"), PointsDetected, EntitiesRevealed);
	OnLidarScanCompleted.Broadcast(PointsDetected, EntitiesRevealed);
}
