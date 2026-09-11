#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LiminalStimulusSubsystem.generated.h"

UENUM(BlueprintType)
enum class ELiminalStimulusType : uint8
{
	Noise,
	Light
};

USTRUCT(BlueprintType)
struct MEG_RECLAMATION_API FLiminalStimulus
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Stimulus")
	ELiminalStimulusType Type = ELiminalStimulusType::Noise;

	UPROPERTY(BlueprintReadOnly, Category = "Stimulus")
	FVector Location = FVector::ZeroVector;

	/** Intensite normalisee 0..1 (RMS pour le bruit, lumens normalises pour la lumiere). */
	UPROPERTY(BlueprintReadOnly, Category = "Stimulus")
	float Strength = 0.0f;

	/** Portee de perception en centimetres. */
	UPROPERTY(BlueprintReadOnly, Category = "Stimulus")
	float RadiusCm = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Stimulus")
	float ServerTimeSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Stimulus")
	TWeakObjectPtr<AActor> Instigator;

	bool IsInRange(const FVector& ListenerLocation) const;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLiminalStimulusReported, const FLiminalStimulus&);

/**
 * Bus de stimuli perceptifs pour le bestiaire. Serveur uniquement : les outils appellent
 * un Server RPC sur leur pawn owner, le serveur emet le stimulus ici, puis un Multicast
 * cosmetique vers les clients. Aucun client ne peut injecter un stimulus.
 */
UCLASS()
class MEG_RECLAMATION_API ULiminalStimulusSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Bruit : LoudnessRms 0..1. Ignore sous NoiseRmsThreshold. Retourne true si enregistre. */
	UFUNCTION(BlueprintCallable, Category = "Stimulus")
	bool ReportNoise(const FVector& Location, float LoudnessRms, AActor* Instigator);

	/** Lumiere : Intensity01 0..1. Retourne true si enregistre. */
	UFUNCTION(BlueprintCallable, Category = "Stimulus")
	bool ReportLight(const FVector& Location, float Intensity01, AActor* Instigator);

	/** Stimulus le plus fort du type donne, encore valide et a portee du point d'ecoute. */
	UFUNCTION(BlueprintPure, Category = "Stimulus")
	bool HasStimulusInRange(ELiminalStimulusType Type, const FVector& ListenerLocation, float MaxAgeSeconds, FLiminalStimulus& OutStrongest) const;

	UFUNCTION(BlueprintPure, Category = "Stimulus")
	int32 GetRecentStimuli(ELiminalStimulusType Type, float MaxAgeSeconds, TArray<FLiminalStimulus>& OutStimuli) const;

	UFUNCTION(BlueprintCallable, Category = "Stimulus")
	void ClearAll();

	/** Abonnement natif pour les AIControllers (reaction immediate sans polling). */
	FOnLiminalStimulusReported OnStimulusReported;

	/** Seuil RMS du GDD : A_RMS > 0.03 => alerte du Hound. */
	static constexpr float NoiseRmsThreshold = 0.03f;
	static constexpr float MaxNoiseRadiusCm = 6000.0f;
	static constexpr float MaxLightRadiusCm = 4000.0f;
	static constexpr float StimulusLifetimeSeconds = 10.0f;
	static constexpr int32 MaxStoredStimuli = 64;

private:
	bool IsServerAuthority() const;
	float GetNowSeconds() const;
	void Prune(float NowSeconds);
	bool Push(FLiminalStimulus&& Stimulus);

	TArray<FLiminalStimulus> Stimuli;
};
