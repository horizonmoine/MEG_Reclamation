#pragma once

#include "CoreMinimal.h"
#include "Tools/BaseTool.h"
#include "LidarScannerTool.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLidarScanCompleted, int32, PointsDetected, int32, EntitiesRevealed);

/**
 * Scanner LIDAR (Outil Phase 2 - Etape 11 de la Roadmap).
 * Projette des impulsions de scan volumétrique révélant la topologie dans le noir complet
 * et perçant le camouflage optique des entités furtives (Dullers).
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ALidarScannerTool : public ABaseTool
{
	GENERATED_BODY()

public:
	ALidarScannerTool();

	virtual bool Activate() override;
	virtual bool CanActivate() const override;

	UFUNCTION(BlueprintPure, Category = "LIDAR")
	float GetScanRange() const { return ScanRange; }

	UFUNCTION(BlueprintPure, Category = "LIDAR")
	float GetScanConeAngle() const { return ScanConeAngle; }

	UPROPERTY(BlueprintAssignable, Category = "LIDAR")
	FOnLidarScanCompleted OnLidarScanCompleted;

protected:
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LIDAR", meta = (ClampMin = "300.0"))
	float ScanRange = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LIDAR", meta = (ClampMin = "10.0", ClampMax = "120.0"))
	float ScanConeAngle = 65.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LIDAR", meta = (ClampMin = "1.0"))
	float BatteryCostPerPulse = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LIDAR", meta = (ClampMin = "0.2"))
	float ScanCooldownSeconds = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LIDAR", meta = (ClampMin = "1.0"))
	float DullerRevealDuration = 8.0f;

private:
	void ExecuteLidarSweep();

	float CooldownTimer = 0.0f;
};
