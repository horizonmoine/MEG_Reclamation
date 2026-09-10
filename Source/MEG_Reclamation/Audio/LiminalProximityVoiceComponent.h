#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LiminalProximityVoiceComponent.generated.h"

class UAudioComponent;
class USoundAttenuation;
class USoundSubmix;
class AScavengerCharacter;

UENUM(BlueprintType)
enum class EVoiceTransmissionMode : uint8
{
	ProximityOnly UMETA(DisplayName = "Proximity Acoustic"),
	RadioBroadcast UMETA(DisplayName = "Walkie Talkie UHF"),
	Muted UMETA(DisplayName = "Muted / Silent")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVoiceSpeakingStateChanged, AScavengerCharacter*, Speaker, bool, bIsSpeaking, EVoiceTransmissionMode, Mode);

/**
 * Composant de chat vocal de proximite diegetique avec occlusion murale et relais radio.
 * Gere la spatialisation sonore 3D, le filtrage passe-bas a travers les cloisons (low-pass filter),
 * et l'integration avec le composant de mimicry vocale du Skinwalker.
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class MEG_RECLAMATION_API ULiminalProximityVoiceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULiminalProximityVoiceComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Active ou desactive la transmission vocale (PTT) */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Voice")
	void SetTransmitting(bool bTransmitting, EVoiceTransmissionMode Mode = EVoiceTransmissionMode::ProximityOnly);

	/** Calcule l'occlusion acoustique entre ce speaker et un auditeur (0.0 = vue directe claire, 1.0 = occlus par 2+ murs) */
	UFUNCTION(BlueprintPure, Category = "Liminal|Voice")
	float CalculateWallOcclusionTo(const FVector& ListenerLocation) const;

	/** Verifie si l'auditeur peut entendre ce speaker a cette position */
	UFUNCTION(BlueprintPure, Category = "Liminal|Voice")
	bool CanBeHeardBy(const FVector& ListenerLocation, float& OutEffectiveVolume, float& OutLowPassCutoffHz) const;

	UFUNCTION(BlueprintPure, Category = "Liminal|Voice")
	bool IsSpeaking() const { return bIsSpeaking; }

	UFUNCTION(BlueprintPure, Category = "Liminal|Voice")
	EVoiceTransmissionMode GetCurrentTransmissionMode() const { return CurrentMode; }

	/** Current RMS loudness from the microphone capture (0.0 to 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Liminal|Voice")
	float CurrentAcousticRMS = 0.0f;

	UPROPERTY(BlueprintAssignable, Category = "Liminal|Voice")
	FOnVoiceSpeakingStateChanged OnVoiceSpeakingStateChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Liminal|Voice|Attenuation", meta = (ClampMin = "100.0"))
	float ProximityInnerRadius = 300.0f; // 3m pleine puissance

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Liminal|Voice|Attenuation", meta = (ClampMin = "500.0"))
	float ProximityMaxHearingRadius = 2500.0f; // 25m seuil d'audibilite

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Liminal|Voice|Occlusion")
	float SingleWallVolumeLoss = 0.45f; // 45% de perte par mur traverse

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Liminal|Voice|Occlusion")
	float LowPassFrequencyPerWallHz = 850.0f; // Muffle sourd a travers les cloisons

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Liminal|Voice|Occlusion")
	float DirectLineFrequencyHz = 20000.0f; // Voix claire non filtre

	UPROPERTY(ReplicatedUsing = OnRep_VoiceState)
	bool bIsSpeaking = false;

	UPROPERTY(ReplicatedUsing = OnRep_VoiceState)
	EVoiceTransmissionMode CurrentMode = EVoiceTransmissionMode::ProximityOnly;

	UFUNCTION()
	void OnRep_VoiceState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> VoiceAudioComponent;
};
