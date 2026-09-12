#include "Sanity/LiminalSanityPostProcessComponent.h"
#include "Sanity/LiminalHallucinationActor.h"
#include "Camera/CameraComponent.h"
#include "Components/PostProcessComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameModes/LiminalZoneRulesSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

ULiminalSanityPostProcessComponent::ULiminalSanityPostProcessComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	NextHallucinationInterval = FMath::FRandRange(HallucinationSpawnIntervalMin, HallucinationSpawnIntervalMax);

	static ConstructorHelpers::FObjectFinder<USoundBase> HeartbeatFinder(
		TEXT("/Game/Audio/S_Heartbeat_Panic.S_Heartbeat_Panic"));
	if (HeartbeatFinder.Succeeded())
	{
		HeartbeatSound = HeartbeatFinder.Object;
	}
}

void ULiminalSanityPostProcessComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		CachedCamera = Owner->FindComponentByClass<UCameraComponent>();
		LocalPostProcess = Owner->FindComponentByClass<UPostProcessComponent>();
		if (CachedCamera.IsValid())
		{
			BaseFOV = CachedCamera->FieldOfView;
		}
	}
}

void ULiminalSanityPostProcessComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Only simulate for local client
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	APawn* PawnOwner = Cast<APawn>(Owner);
	if (PawnOwner && !PawnOwner->IsLocallyControlled())
	{
		return;
	}

	ApplyDistortionToCamera(DeltaTime);
	UpdateHallucinationSpawning(DeltaTime);

	// Battement de coeur diegetique quand la sanite vacille
	if (CurrentTier == ESanityTier::Paranoid || CurrentTier == ESanityTier::Psychotic)
	{
		HeartbeatTimer -= DeltaTime;
		if (HeartbeatTimer <= 0.0f)
		{
			const float Interval = (CurrentTier == ESanityTier::Psychotic) ? 0.52f : 0.90f;
			HeartbeatTimer = Interval;
			if (HeartbeatSound && GetWorld())
			{
				const float Vol = (CurrentTier == ESanityTier::Psychotic) ? 0.95f : 0.55f;
				UGameplayStatics::PlaySound2D(GetWorld(), HeartbeatSound, Vol, 1.0f);
			}
		}
	}
	else
	{
		HeartbeatTimer = 0.0f;
	}
}

void ULiminalSanityPostProcessComponent::UpdateSanityState(float CurrentSanity, float MaxSanity)
{
	const float SafeMax = FMath::Max(1.0f, MaxSanity);
	CachedSanityPercent = FMath::Clamp(CurrentSanity / SafeMax, 0.0f, 1.0f);

	const ESanityTier NewTier = LiminalSanity::GetTierFromPercent(CachedSanityPercent);
	if (NewTier != CurrentTier)
	{
		CurrentTier = NewTier;
		OnSanityTierChanged.Broadcast(CurrentTier, CachedSanityPercent);
	}

	if (CurrentTier == ESanityTier::Paranoid)
	{
		TargetVignette = 0.4f;
		TargetChromaticAberration = 1.0f;
	}
	else if (CurrentTier == ESanityTier::Psychotic)
	{
		TargetVignette = 0.6f;
		TargetChromaticAberration = 3.0f;
	}
	else
	{
		TargetVignette = 0.0f;
		TargetChromaticAberration = 0.0f;
	}
}

void ULiminalSanityPostProcessComponent::ApplyDistortionToCamera(float DeltaTime)
{
	if (CurrentTier == ESanityTier::Stable)
	{
		FOVPulseOffset = 0.0f;
		if (CachedCamera.IsValid())
		{
			CachedCamera->SetFieldOfView(BaseFOV);
			CachedCamera->PostProcessSettings.bOverride_SceneFringeIntensity = false;
			CachedCamera->PostProcessSettings.bOverride_VignetteIntensity = false;
			CachedCamera->PostProcessSettings.bOverride_ColorSaturation = false;
		}
		return;
	}

	const float Frequency = (CurrentTier == ESanityTier::Psychotic) ? 3.5f : 1.0f;
	FOVPulseTimer += DeltaTime * Frequency;
	const float Amplitude = (CurrentTier == ESanityTier::Psychotic) ? 2.0f : 0.0f;

	FOVPulseOffset = FMath::Sin(FOVPulseTimer) * Amplitude;

	if (CachedCamera.IsValid())
	{
		CachedCamera->SetFieldOfView(BaseFOV + FOVPulseOffset);

		CachedCamera->PostProcessSettings.bOverride_SceneFringeIntensity = true;
		CachedCamera->PostProcessSettings.SceneFringeIntensity = TargetChromaticAberration;

		CachedCamera->PostProcessSettings.bOverride_VignetteIntensity = true;
		CachedCamera->PostProcessSettings.VignetteIntensity = TargetVignette;

		if (CurrentTier == ESanityTier::Psychotic)
		{
			CachedCamera->PostProcessSettings.bOverride_ColorSaturation = true;
			CachedCamera->PostProcessSettings.ColorSaturation = FVector4(0.5f, 0.5f, 0.5f, 1.0f);
		}
		else
		{
			CachedCamera->PostProcessSettings.bOverride_ColorSaturation = false;
		}
	}
}

void ULiminalSanityPostProcessComponent::UpdateHallucinationSpawning(float DeltaTime)
{
	if (CurrentTier != ESanityTier::Paranoid && CurrentTier != ESanityTier::Psychotic)
	{
		return;
	}

	if (!ULiminalZoneRulesSubsystem::IsSanityPressureActive(GetOwner()))
	{
		return;
	}

	HallucinationTimer += DeltaTime;
	if (HallucinationTimer >= NextHallucinationInterval)
	{
		HallucinationTimer = 0.0f;
		NextHallucinationInterval = FMath::FRandRange(HallucinationSpawnIntervalMin, HallucinationSpawnIntervalMax);
		if (CurrentTier == ESanityTier::Psychotic)
		{
			NextHallucinationInterval *= 0.6f; // Faster hallucinations when psychotic
		}

		const EHallucinationType PickedType = (FMath::FRand() > 0.5f) ?
			EHallucinationType::ShadowSilhouette :
			(FMath::FRand() > 0.5f ? EHallucinationType::FakeDoor : EHallucinationType::FakeLoot);

		ForceSpawnHallucination(PickedType);
	}
}

void ULiminalSanityPostProcessComponent::ForceSpawnHallucination(EHallucinationType InType)
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return;
	}

	if (!ULiminalZoneRulesSubsystem::IsSanityPressureActive(Owner))
	{
		return;
	}

	const FVector OwnerLocation = Owner->GetActorLocation();
	const FVector ForwardVector = Owner->GetActorForwardVector();

	// Place hallucination ahead, slightly to the side in the dark
	const float AngleDeg = FMath::FRandRange(-45.0f, 45.0f);
	const FVector SpawnDir = ForwardVector.RotateAngleAxis(AngleDeg, FVector::UpVector);
	const FVector SpawnLocation = OwnerLocation + (SpawnDir * HallucinationSpawnDistance);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (ALiminalHallucinationActor* Hallucination = World->SpawnActor<ALiminalHallucinationActor>(
		ALiminalHallucinationActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams))
	{
		Hallucination->ConfigureHallucination(InType, 15.0f);
		UE_LOG(LogTemp, Verbose, TEXT("[LiminalSanity] Spawned client-side hallucination of type %d"),
			static_cast<int32>(InType));
	}
}
