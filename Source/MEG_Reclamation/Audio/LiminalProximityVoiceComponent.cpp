#include "Audio/LiminalProximityVoiceComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AISense_Hearing.h"
#include "Player/ScavengerCharacter.h"

ULiminalProximityVoiceComponent::ULiminalProximityVoiceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
	SetIsReplicatedByDefault(true);
}

void ULiminalProximityVoiceComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULiminalProximityVoiceComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULiminalProximityVoiceComponent, bIsSpeaking);
	DOREPLIFETIME(ULiminalProximityVoiceComponent, CurrentMode);
}

void ULiminalProximityVoiceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Alertes diegetiques IA : Si le joueur parle, les entites auditives (Hounds) le detectent
	if (bIsSpeaking && GetOwner() && GetOwner()->HasAuthority())
	{
		const float NoiseLoudness = (CurrentMode == EVoiceTransmissionMode::RadioBroadcast) ? 0.4f : 0.85f;
		const float HearingRange = (CurrentMode == EVoiceTransmissionMode::RadioBroadcast) ? 800.0f : 1800.0f;

		UAISense_Hearing::ReportNoiseEvent(
			GetWorld(),
			GetOwner()->GetActorLocation(),
			NoiseLoudness,
			GetOwner(),
			HearingRange,
			TEXT("PlayerVoiceSpeech")
		);
	}
}

void ULiminalProximityVoiceComponent::SetTransmitting(bool bTransmitting, EVoiceTransmissionMode Mode)
{
	if (bIsSpeaking == bTransmitting && CurrentMode == Mode)
	{
		return;
	}

	bIsSpeaking = bTransmitting;
	CurrentMode = Mode;

	OnRep_VoiceState();
}

void ULiminalProximityVoiceComponent::OnRep_VoiceState()
{
	AScavengerCharacter* Scavenger = Cast<AScavengerCharacter>(GetOwner());
	OnVoiceSpeakingStateChanged.Broadcast(Scavenger, bIsSpeaking, CurrentMode);
}

float ULiminalProximityVoiceComponent::CalculateWallOcclusionTo(const FVector& ListenerLocation) const
{
	UWorld* World = GetWorld();
	if (!World || !GetOwner())
	{
		return 0.0f;
	}

	const FVector SpeakerHeadLoc = GetOwner()->GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	TArray<FHitResult> HitResults;
	World->LineTraceMultiByChannel(HitResults, SpeakerHeadLoc, ListenerLocation, ECC_Visibility, Params);

	int32 WallCount = 0;
	for (const FHitResult& Hit : HitResults)
	{
		if (Hit.bBlockingHit && Hit.GetActor() != GetOwner())
		{
			WallCount++;
		}
	}

	return FMath::Clamp(WallCount * SingleWallVolumeLoss, 0.0f, 1.0f);
}

bool ULiminalProximityVoiceComponent::CanBeHeardBy(const FVector& ListenerLocation, float& OutEffectiveVolume, float& OutLowPassCutoffHz) const
{
	OutEffectiveVolume = 0.0f;
	OutLowPassCutoffHz = DirectLineFrequencyHz;

	if (!bIsSpeaking || CurrentMode == EVoiceTransmissionMode::Muted || !GetOwner())
	{
		return false;
	}

	// Mode Radio UHF : propagation directe sans perte de distance spatiale
	if (CurrentMode == EVoiceTransmissionMode::RadioBroadcast)
	{
		OutEffectiveVolume = 1.0f;
		OutLowPassCutoffHz = 3800.0f; // Filtre bandpass type radio talkie
		return true;
	}

	// Mode Proximite Acoustique : attenuation spatiale et occlusion murale
	const FVector SpeakerLoc = GetOwner()->GetActorLocation();
	const float Distance = FVector::Dist(SpeakerLoc, ListenerLocation);

	if (Distance > ProximityMaxHearingRadius)
	{
		return false;
	}

	float DistanceAttenuation = 1.0f;
	if (Distance > ProximityInnerRadius)
	{
		const float Alpha = (Distance - ProximityInnerRadius) / (ProximityMaxHearingRadius - ProximityInnerRadius);
		DistanceAttenuation = 1.0f - FMath::Clamp(Alpha, 0.0f, 1.0f);
	}

	const float Occlusion = CalculateWallOcclusionTo(ListenerLocation);
	OutEffectiveVolume = DistanceAttenuation * (1.0f - Occlusion);

	if (OutEffectiveVolume <= 0.01f)
	{
		return false;
	}

	OutLowPassCutoffHz = FMath::Lerp(DirectLineFrequencyHz, LowPassFrequencyPerWallHz, Occlusion);
	return true;
}
