#pragma once

#include "CoreMinimal.h"
#include "Tools/BaseTool.h"
#include "SignalAnalyzerTool.generated.h"

USTRUCT(BlueprintType)
struct MEG_RECLAMATION_API FSignalReading
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Signal")
	bool bSignalDetected = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Signal")
	float SignalStrength01 = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Signal")
	float DistanceUnits = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Signal")
	FName SignalCategory = NAME_None; // "Loot", "Anomaly", "Extraction"
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSignalAnalyzed, const FSignalReading&, Reading);

/**
 * Analyseur de Fréquence / Signal (Outil Phase 2 - Etape 11 de la Roadmap).
 * Console portable diégétique avec spectrographe radio permettant de localiser
 * les artefacts, les anomalies et le butin à travers les cloisons.
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ASignalAnalyzerTool : public ABaseTool
{
	GENERATED_BODY()

public:
	ASignalAnalyzerTool();

	virtual bool Activate() override;
	virtual bool CanActivate() const override;

	UFUNCTION(BlueprintCallable, Category = "Signal")
	bool PerformSpectrographicScan(FSignalReading& OutReading);

	UFUNCTION(BlueprintPure, Category = "Signal")
	const FSignalReading& GetLastReading() const { return LastReading; }

	UPROPERTY(BlueprintAssignable, Category = "Signal")
	FOnSignalAnalyzed OnSignalAnalyzed;

protected:
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Signal", meta = (ClampMin = "500.0"))
	float DetectionRadius = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Signal", meta = (ClampMin = "1.0"))
	float ScanBatteryCost = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Signal", meta = (ClampMin = "0.2"))
	float ScanCooldownSeconds = 0.8f;

private:
	FSignalReading LastReading;
	float CooldownTimer = 0.0f;
};
