#include "Player/LiminalBodycamComponent.h"
#include "Player/ScavengerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ULiminalBodycamComponent::ULiminalBodycamComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULiminalBodycamComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AScavengerCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		OwnerCamera = OwnerCharacter->FindComponentByClass<UCameraComponent>();
		if (OwnerCamera)
		{
			PreviousCameraRot = OwnerCamera->GetComponentRotation();
		}
	}

	ApplyPostProcess();
}

void ULiminalBodycamComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateTelemetry(DeltaTime);
	ApplyPostProcess();
}

void ULiminalBodycamComponent::ToggleNightVision()
{
	SetNightVision(!TelemetryData.bNightVisionActive);
}

void ULiminalBodycamComponent::SetNightVision(bool bEnable)
{
	if (bEnable && TelemetryData.BatteryPercent <= 0.0f)
	{
		TelemetryData.bNightVisionActive = false;
	}
	else
	{
		TelemetryData.bNightVisionActive = bEnable;
	}
	ApplyPostProcess();
}

void ULiminalBodycamComponent::CalculateBodycamOffset(float DeltaTime, FVector& OutLocationOffset, FRotator& OutRotationOffset)
{
	OutLocationOffset = FVector::ZeroVector;
	OutRotationOffset = FRotator::ZeroRotator;

	if (!OwnerCharacter)
	{
		return;
	}

	const float Speed = OwnerCharacter->GetVelocity().Size2D();
	const bool bIsMoving = Speed > 10.0f;

	if (bIsMoving)
	{
		const float BobFrequency = (Speed > 350.0f) ? 9.5f : 6.0f; // Sprint vs Marche
		const float BobAmplitudeZ = (Speed > 350.0f) ? 3.5f : 1.5f;
		const float BobAmplitudeY = (Speed > 350.0f) ? 2.5f : 1.0f;

		HeadbobTimer += DeltaTime * BobFrequency;

		OutLocationOffset.Z = FMath::Sin(HeadbobTimer) * BobAmplitudeZ;
		OutLocationOffset.Y = FMath::Cos(HeadbobTimer * 0.5f) * BobAmplitudeY;

		// Roll et Pitch subtils simulant le port de la camera sur le harnais d'epaule
		OutRotationOffset.Roll = FMath::Cos(HeadbobTimer * 0.5f) * (BobAmplitudeY * 0.4f);
		OutRotationOffset.Pitch = FMath::Sin(HeadbobTimer) * (BobAmplitudeZ * 0.25f);
	}
	else
	{
		// Respiration au repos
		HeadbobTimer += DeltaTime * 1.5f;
		OutLocationOffset.Z = FMath::Sin(HeadbobTimer) * 0.35f;
	}
}

void ULiminalBodycamComponent::UpdateTelemetry(float DeltaTime)
{
	SimSecondsElapsed += DeltaTime;

	// Consommation batterie si vision nocturne active
	if (TelemetryData.bNightVisionActive)
	{
		TelemetryData.BatteryPercent = FMath::Max(0.0f, TelemetryData.BatteryPercent - (NightVisionBatteryDrainPerSec * DeltaTime));
		if (TelemetryData.BatteryPercent <= 0.0f)
		{
			SetNightVision(false);
		}
	}

	// Calcul de l'horodatage 1998 simule
	const int32 BaseSeconds = 13335 + FMath::FloorToInt(SimSecondsElapsed); // Commence a 03:42:15
	const int32 Hours = (BaseSeconds / 3600) % 24;
	const int32 Minutes = (BaseSeconds / 60) % 60;
	const int32 Seconds = BaseSeconds % 60;
	TelemetryData.TimestampString = FString::Printf(TEXT("1998-10-24 %02d:%02d:%02d"), Hours, Minutes, Seconds);

	// Frequence cardiaque dynamique BPM basee sur endurance et sante mentale
	float StaminaNorm = 1.0f;
	float SanityNorm = 1.0f;
	if (OwnerCharacter)
	{
		StaminaNorm = OwnerCharacter->GetStaminaPercent();
		SanityNorm = OwnerCharacter->GetSanityPercent();
	}

	const float FatigueFactor = (1.0f - StaminaNorm) * 60.0f;
	const float PanicFactor = (1.0f - SanityNorm) * 50.0f;
	TelemetryData.HeartRateBPM = FMath::Clamp(FMath::RoundToInt(72.0f + FatigueFactor + PanicFactor), 60, 195);

	if (SanityNorm < 0.3f)
	{
		TelemetryData.StatusWarning = TEXT("CRITICAL SANITY LOSS");
	}
	else if (StaminaNorm < 0.2f)
	{
		TelemetryData.StatusWarning = TEXT("EXHAUSTION WARNING");
	}
	else
	{
		TelemetryData.StatusWarning = TEXT("NORMAL");
	}
}

void ULiminalBodycamComponent::ApplyPostProcess()
{
	if (!OwnerCamera || !bEnableBodycamPostProcess)
	{
		return;
	}

	FPostProcessSettings& PPS = OwnerCamera->PostProcessSettings;

	// Reglage du vignettage type objectif camera portative
	PPS.bOverride_VignetteIntensity = true;
	PPS.VignetteIntensity = VignetteIntensity;

	// Aberration chromatique (dispersion aux bords de la lentille)
	PPS.bOverride_SceneFringeIntensity = true;
	PPS.SceneFringeIntensity = ChromaticAberration;

	// Grain d'image 16mm / VHS
	PPS.bOverride_FilmGrainIntensity = true;
	PPS.FilmGrainIntensity = FilmGrainIntensity;

	// Mode Vision Nocturne IR
	if (TelemetryData.bNightVisionActive)
	{
		PPS.bOverride_ColorSaturation = true;
		PPS.ColorSaturation = FVector4(0.1f, 1.4f, 0.2f, 1.0f); // Teinte verdoyante phosphore

		PPS.bOverride_AutoExposureBias = true;
		PPS.AutoExposureBias = 2.5f; // Amplification de luminosite

		PPS.bOverride_FilmGrainIntensity = true;
		PPS.FilmGrainIntensity = FilmGrainIntensity * 2.2f; // Plus de bruit thermique
	}
	else
	{
		PPS.bOverride_ColorSaturation = false;
		PPS.bOverride_AutoExposureBias = false;
	}
}
