#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LiminalBodycamComponent.generated.h"

class UCameraComponent;
class AScavengerCharacter;

USTRUCT(BlueprintType)
struct FBodycamTelemetryData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Bodycam")
	FString TimestampString = TEXT("1998-10-24 03:42:15");

	UPROPERTY(BlueprintReadOnly, Category = "Bodycam")
	int32 HeartRateBPM = 72;

	UPROPERTY(BlueprintReadOnly, Category = "Bodycam")
	float BatteryPercent = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Bodycam")
	FString OperativeID = TEXT("M.E.G. REC-OP #04");

	UPROPERTY(BlueprintReadOnly, Category = "Bodycam")
	FString StatusWarning = TEXT("NORMAL");

	UPROPERTY(BlueprintReadOnly, Category = "Bodycam")
	bool bNightVisionActive = false;
};

/**
 * Composant de camera d'epaule / bodycam 1998 M.E.G.
 * Gere :
 * - Le headbob / cam sway physique et l'inertie du corps
 * - Les reglages Post-Process (vignettage, aberration chromatique, grain argentique 16mm/VHS)
 * - La telemetrie diagétique (Horodatage 1998, BPM cardiaque calé sur la stamina/panique)
 * - Le mode vision nocturne IR (Infra-Rouge)
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MEG_RECLAMATION_API ULiminalBodycamComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULiminalBodycamComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Active ou desactive la vision nocturne IR */
	UFUNCTION(BlueprintCallable, Category = "Bodycam")
	void ToggleNightVision();

	UFUNCTION(BlueprintCallable, Category = "Bodycam")
	void SetNightVision(bool bEnable);

	UFUNCTION(BlueprintPure, Category = "Bodycam")
	const FBodycamTelemetryData& GetTelemetryData() const { return TelemetryData; }

	/** Calcule l'offset rotationnel et positionnel du bodycam sway a appliquer a la camera */
	UFUNCTION(BlueprintCallable, Category = "Bodycam")
	void CalculateBodycamOffset(float DeltaTime, FVector& OutLocationOffset, FRotator& OutRotationOffset);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bodycam|Settings")
	bool bEnableBodycamPostProcess = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bodycam|Settings")
	float VignetteIntensity = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bodycam|Settings")
	float ChromaticAberration = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bodycam|Settings")
	float FilmGrainIntensity = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bodycam|Settings")
	float BarrelDistortionIntensity = 0.15f;

protected:
	virtual void BeginPlay() override;

private:
	void UpdateTelemetry(float DeltaTime);
	void ApplyPostProcess();

	UPROPERTY()
	TObjectPtr<AScavengerCharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UCameraComponent> OwnerCamera;

	FBodycamTelemetryData TelemetryData;

	float SimSecondsElapsed = 0.0f;
	float HeadbobTimer = 0.0f;
	float NightVisionBatteryDrainPerSec = 1.5f;

	// Sway & Inertie
	FRotator PreviousCameraRot = FRotator::ZeroRotator;
	FRotator SmoothedSwayRot = FRotator::ZeroRotator;
};
