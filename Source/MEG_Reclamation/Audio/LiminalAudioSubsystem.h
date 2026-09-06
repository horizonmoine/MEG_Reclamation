#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/LiminalGameInstance.h"
#include "LiminalAudioSubsystem.generated.h"

class UAudioComponent;
class USoundBase;

/**
 * Profil acoustique et sonore d'un biome liminal.
 */
USTRUCT(BlueprintType)
struct MEG_RECLAMATION_API FLiminalBiomeAudioProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Biome")
	ELevelBiome Biome = ELevelBiome::Level0_YellowLobby;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Biome")
	FName AmbientLoopId = FName("Amb_YellowLobby_60HzHum");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Biome", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float AmbientVolume = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Biome", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float DronePitch = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Reverb", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float ReverbDecaySeconds = 2.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Reverb", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ReverbDamping = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Filter", meta = (ClampMin = "100.0", ClampMax = "22000.0"))
	float LowPassCutoffHz = 20000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Stingers", meta = (ClampMin = "1.0"))
	float StingerIntervalMin = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Stingers", meta = (ClampMin = "1.0"))
	float StingerIntervalMax = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Stingers")
	TArray<FName> StingerIds;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBiomeAudioChanged, ELevelBiome, NewBiome, const FLiminalBiomeAudioProfile&, Profile);

/**
 * Sous-systeme audio des niveaux liminaux (Etape 9 de la Roadmap).
 * Gere les ambiances sonores spatialisees, la reverberation convolutionnelle
 * selon le biome et la taille des salles, ainsi que les bruits proceduraux.
 */
UCLASS()
class MEG_RECLAMATION_API ULiminalAudioSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	ULiminalAudioSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Liminal|Audio")
	void SetCurrentBiome(ELevelBiome NewBiome);

	UFUNCTION(BlueprintPure, Category = "Liminal|Audio")
	ELevelBiome GetCurrentBiome() const { return CurrentBiome; }

	UFUNCTION(BlueprintPure, Category = "Liminal|Audio")
	const FLiminalBiomeAudioProfile& GetCurrentProfile() const { return ActiveProfile; }

	UFUNCTION(BlueprintPure, Category = "Liminal|Audio")
	FLiminalBiomeAudioProfile GetProfileForBiome(ELevelBiome Biome) const;

	UFUNCTION(BlueprintCallable, Category = "Liminal|Audio")
	void UpdateRoomAcoustics(float RoomVolumeMetersCubed, float WallHardnessFactor = 0.8f);

	UFUNCTION(BlueprintPure, Category = "Liminal|Audio")
	float GetEffectiveReverbDecay() const { return DynamicReverbDecay; }

	UFUNCTION(BlueprintPure, Category = "Liminal|Audio")
	float GetEffectiveLowPassCutoff() const { return DynamicLowPassCutoff; }

	UFUNCTION(BlueprintCallable, Category = "Liminal|Audio")
	void TriggerProceduralStinger(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Liminal|Audio")
	void ProcessVoiceNoise(AActor* Speaker, float InputDecibels);

	UPROPERTY(BlueprintAssignable, Category = "Liminal|Audio")
	FOnBiomeAudioChanged OnBiomeAudioChanged;

private:
	void BuildDefaultProfiles();
	void ScheduleNextStinger();
	void OnStingerTimerExpired();

	ELevelBiome CurrentBiome = ELevelBiome::Level0_YellowLobby;
	FLiminalBiomeAudioProfile ActiveProfile;
	TMap<ELevelBiome, FLiminalBiomeAudioProfile> BiomeProfiles;

	float DynamicReverbDecay = 2.8f;
	float DynamicLowPassCutoff = 20000.0f;

	FTimerHandle StingerTimerHandle;
};
