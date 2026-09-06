#include "Tools/SignalAnalyzerTool.h"
#include "Objects/LootActor.h"
#include "AI/LiminalEntity.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

ASignalAnalyzerTool::ASignalAnalyzerTool()
{
	MaxBatteryCharge = 100.0f;
	BatteryCharge = 100.0f;
	ActiveBatteryDrainPerSecond = 0.0f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SignalFinder(
		TEXT("/Game/Meshes/Tools/SM_Signal_Analyzer.SM_Signal_Analyzer"));
	if (SignalFinder.Succeeded() && ToolMesh)
	{
		ToolMesh->SetStaticMesh(SignalFinder.Object);
		ToolMesh->SetRelativeScale3D(FVector(1.0f));
	}
}

bool ASignalAnalyzerTool::CanActivate() const
{
	return Super::CanActivate() && CooldownTimer <= 0.0f && BatteryCharge >= ScanBatteryCost;
}

bool ASignalAnalyzerTool::Activate()
{
	if (!CanActivate())
	{
		return false;
	}

	SetBatteryCharge(FMath::Max(0.0f, BatteryCharge - ScanBatteryCost));
	CooldownTimer = ScanCooldownSeconds;

	PerformSpectrographicScan(LastReading);
	return true;
}

void ASignalAnalyzerTool::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CooldownTimer > 0.0f)
	{
		CooldownTimer -= DeltaSeconds;
	}

	if (BatteryCharge < MaxBatteryCharge)
	{
		SetBatteryCharge(FMath::Min(MaxBatteryCharge, BatteryCharge + 3.0f * DeltaSeconds));
	}
}

bool ASignalAnalyzerTool::PerformSpectrographicScan(FSignalReading& OutReading)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		OutReading = FSignalReading();
		return false;
	}

	const FVector ScannerPos = GetActorLocation();
	float ClosestDist = DetectionRadius;
	FName DetectedCategory = NAME_None;
	bool bFound = false;

	// 1. Search for closest loot
	for (TActorIterator<ALootActor> It(World); It; ++It)
	{
		ALootActor* Loot = *It;
		if (Loot)
		{
			const float Dist = FVector::Dist(ScannerPos, Loot->GetActorLocation());
			if (Dist < ClosestDist)
			{
				ClosestDist = Dist;
				DetectedCategory = FName("Loot");
				bFound = true;
			}
		}
	}

	// 2. Search for closest entity/anomaly
	for (TActorIterator<ALiminalEntity> It(World); It; ++It)
	{
		ALiminalEntity* Entity = *It;
		if (Entity)
		{
			const float Dist = FVector::Dist(ScannerPos, Entity->GetActorLocation());
			if (Dist < ClosestDist)
			{
				ClosestDist = Dist;
				DetectedCategory = FName("Anomaly");
				bFound = true;
			}
		}
	}

	OutReading.bSignalDetected = bFound;
	OutReading.DistanceUnits = bFound ? ClosestDist : 0.0f;
	OutReading.SignalCategory = DetectedCategory;
	OutReading.SignalStrength01 = bFound ? FMath::Clamp(1.0f - (ClosestDist / DetectionRadius), 0.05f, 1.0f) : 0.0f;

	LastReading = OutReading;
	OnSignalAnalyzed.Broadcast(OutReading);

	UE_LOG(LogTemp, Log, TEXT("[SignalAnalyzer] Scan: Category '%s', Distance %.0f, Strength %.2f"),
		*OutReading.SignalCategory.ToString(), OutReading.DistanceUnits, OutReading.SignalStrength01);

	return bFound;
}
