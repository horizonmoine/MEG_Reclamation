#pragma once

#include "CoreMinimal.h"
#include "AI/LiminalEntity.h"
#include "Perception/AIPerceptionTypes.h"
#include "LiminalEntity_Hound.generated.h"

class AScavengerCharacter;

UENUM(BlueprintType)
enum class EHoundAcousticState : uint8
{
	Attente UMETA(DisplayName = "Attente"),
	Suspicion UMETA(DisplayName = "Suspicion"),
	Recherche UMETA(DisplayName = "Recherche"),
	Poursuite UMETA(DisplayName = "Poursuite"),
	Attaque UMETA(DisplayName = "Attaque"),
	Recuperation UMETA(DisplayName = "Recuperation")
};

USTRUCT(BlueprintType)
struct FHoundAcousticStimulus
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hound|Audio")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hound|Audio")
	float SourceLoudness = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hound|Audio")
	float PerceivedIntensity = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hound|Audio")
	float Timestamp = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hound|Audio")
	TWeakObjectPtr<AActor> Instigator = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hound|Audio")
	bool bIsOccluded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hound|Audio")
	FName StimulusTag = NAME_None;
};

UCLASS()
class MEG_RECLAMATION_API ALiminalEntity_Hound : public ALiminalEntity
{
	GENERATED_BODY()

public:
	ALiminalEntity_Hound();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "Hound|State")
	EHoundAcousticState GetHoundState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Hound|Audio")
	float GetLastPerceivedLoudness() const { return LastPerceivedLoudness; }

	UFUNCTION(BlueprintPure, Category = "Hound|Behavior")
	bool IsTargetIntimidating() const { return bTargetIsIntimidating; }

	UFUNCTION(BlueprintPure, Category = "Hound|Behavior")
	AActor* GetCurrentAcousticTarget() const { return CurrentTargetActor.Get(); }

	UFUNCTION(BlueprintPure, Category = "Hound|Behavior")
	FVector GetLastKnownSoundLocation() const { return LastKnownSoundLocation; }

	UFUNCTION(BlueprintCallable, Category = "Hound|Audio")
	void RegisterAcousticStimulus(const FVector& StimulusLocation, float Loudness, AActor* SourceActor = nullptr, FName Tag = NAME_None);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hound|Audio")
	float CurrentAcousticRMS = 0.0f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Combat")
	float ChargeSpeed = 750.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Combat")
	float NormalStalkSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Audio")
	float RMSTrackingThreshold = 0.03f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Audio")
	float SuspicionThreshold = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Audio")
	float ChargeThreshold = 0.22f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Audio")
	float MaxHearingRange = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Audio")
	float OcclusionFactor = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Behavior")
	float IntimidationRange = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Behavior")
	float DeAggroTime = 3.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Behavior")
	float BoundedSearchDuration = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Behavior")
	float RecoveryDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Audio")
	float StimulusMemoryDuration = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hound|Audio")
	float TargetSwitchHysteresis = 1.25f;

private:
	void UpdateAcousticTracking(float DeltaSeconds);
	void UpdateAcousticStimuliFromEnvironment();
	void SelectBestStimulus(float CurrentTime);
	void CheckEyeContactIntimidation();

	UPROPERTY(VisibleAnywhere, Category = "Hound|State")
	EHoundAcousticState CurrentState = EHoundAcousticState::Attente;

	UPROPERTY(VisibleAnywhere, Category = "Hound|Audio")
	FHoundAcousticStimulus CurrentBestStimulus;

	UPROPERTY(VisibleAnywhere, Category = "Hound|Audio")
	TArray<FHoundAcousticStimulus> StimulusMemory;

	TWeakObjectPtr<AActor> CurrentTargetActor;
	FVector LastKnownSoundLocation = FVector::ZeroVector;

	bool bTargetIsIntimidating = false;
	float SearchTimer = 0.0f;
	float RecoveryTimer = 0.0f;
	float PursuitLostTimer = 0.0f;
	float LastPerceivedLoudness = 0.0f;
};
