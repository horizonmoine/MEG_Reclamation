#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sanity/LiminalSanityTypes.h"
#include "LiminalSanityPostProcessComponent.generated.h"

class UPostProcessComponent;
class UCameraComponent;
class ALiminalHallucinationActor;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSanityTierChanged, ESanityTier, NewTier, float, SanityPercent);

/**
 * Composant client-side gerant le Post-Process de distortion et les hallucinations
 * reactives selon le niveau de sanite mentale (Etape 10 de la Roadmap).
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MEG_RECLAMATION_API ULiminalSanityPostProcessComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULiminalSanityPostProcessComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Sanity|Feedback")
	void UpdateSanityState(float CurrentSanity, float MaxSanity);

	UFUNCTION(BlueprintPure, Category = "Sanity|Feedback")
	ESanityTier GetCurrentTier() const { return CurrentTier; }

	UFUNCTION(BlueprintPure, Category = "Sanity|Feedback")
	float GetCurrentChromaticAberration() const { return TargetChromaticAberration; }

	UFUNCTION(BlueprintPure, Category = "Sanity|Feedback")
	float GetCurrentVignette() const { return TargetVignette; }

	UFUNCTION(BlueprintPure, Category = "Sanity|Feedback")
	float GetFOVPulseOffset() const { return FOVPulseOffset; }

	UFUNCTION(BlueprintCallable, Category = "Sanity|Hallucination")
	void ForceSpawnHallucination(EHallucinationType InType);

	UPROPERTY(BlueprintAssignable, Category = "Sanity|Feedback")
	FOnSanityTierChanged OnSanityTierChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sanity|PostProcess")
	float MaxChromaticAberration = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sanity|PostProcess")
	float MaxVignetteIntensity = 0.7f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sanity|PostProcess")
	float MaxFOVPulseAmplitude = 6.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sanity|PostProcess")
	float HallucinationSpawnIntervalMin = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sanity|PostProcess")
	float HallucinationSpawnIntervalMax = 18.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sanity|PostProcess")
	float HallucinationSpawnDistance = 900.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sanity|Audio")
	TObjectPtr<USoundBase> HeartbeatSound;

private:
	void ApplyDistortionToCamera(float DeltaTime);
	void UpdateHallucinationSpawning(float DeltaTime);

	ESanityTier CurrentTier = ESanityTier::Stable;
	float CachedSanityPercent = 1.0f;

	float TargetChromaticAberration = 0.0f;
	float TargetVignette = 0.0f;
	float FOVPulseOffset = 0.0f;
	float FOVPulseTimer = 0.0f;
	float BaseFOV = 90.0f;

	float HeartbeatTimer = 0.0f;
	float HallucinationTimer = 0.0f;
	float NextHallucinationInterval = 12.0f;

	TWeakObjectPtr<UCameraComponent> CachedCamera;
	TWeakObjectPtr<UPostProcessComponent> LocalPostProcess;
};
