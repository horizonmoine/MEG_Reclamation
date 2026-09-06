#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HAL/CriticalSection.h"
#include "VoiceMimicryComponent.generated.h"

/**
 * Structure d'un fragment vocal mémorisé en RAM.
 */
USTRUCT(BlueprintType)
struct MEG_RECLAMATION_API FVoiceSnippet
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voice")
	FName SpeakerId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voice")
	float DurationSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voice")
	float Timestamp = 0.0f;

	// Nombre d'échantillons PCM valides dans le buffer circulaire
	int32 SampleCount = 0;
	int32 BufferStartIndex = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceSnippetRecorded, const FVoiceSnippet&, Snippet);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceMimicked, FName, SpeakerId, const FVector&, PlaybackLocation);

/**
 * Composant de mimétisme vocal pour le Skinwalker (Etape 11 de la Roadmap).
 *
 * Buffer circulaire 100% en RAM, jamais écrit sur disque.
 * Thread-safety absolue pour le thread audio : buffers pré-alloués et FCriticalSection.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MEG_RECLAMATION_API UVoiceMimicryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoiceMimicryComponent();

	virtual void BeginPlay() override;

	/** Enregistre un bloc d'échantillons audio PCM (thread-safe, zéro allocation dynamique). */
	bool PushAudioSamples(const float* InSamples, int32 NumSamples, FName SpeakerId);

	/** Récupère le dernier fragment vocal mémorisé. */
	bool GetLatestSnippet(FVoiceSnippet& OutSnippet) const;

	/** Simule la capture d'une réplique d'un allié (utilisé pour les bots ou tests). */
	UFUNCTION(BlueprintCallable, Category = "Voice|Mimicry")
	void SimulateCaptureVoiceSnippet(FName SpeakerName, float DurationSeconds);

	/** Déclenche la réémission d'un fragment mémorisé pour attirer une cible. */
	UFUNCTION(BlueprintCallable, Category = "Voice|Mimicry")
	bool TriggerMimicryCall(const FVector& EmissionLocation, FVoiceSnippet& OutSnippetUsed);

	UFUNCTION(BlueprintPure, Category = "Voice|Mimicry")
	int32 GetStoredSnippetCount() const;

	UFUNCTION(BlueprintPure, Category = "Voice|Mimicry")
	int32 GetMaxSampleCapacity() const { return MaxBufferSamples; }

	UFUNCTION(BlueprintPure, Category = "Voice|Mimicry")
	bool HasAnyRecordedVoices() const;

	UPROPERTY(BlueprintAssignable, Category = "Voice|Mimicry")
	FOnVoiceSnippetRecorded OnVoiceSnippetRecorded;

	UPROPERTY(BlueprintAssignable, Category = "Voice|Mimicry")
	FOnVoiceMimicked OnVoiceMimicked;

private:
	// Capacité maximale du buffer circulaire : 5 secondes à 16 000 Hz = 80 000 échantillons
	static constexpr int32 MaxBufferSamples = 80000;
	static constexpr int32 MaxStoredSnippets = 8;

	// Buffer audio circulaire pré-alloué en mémoire
	TArray<float> CircularSampleBuffer;
	int32 WriteHeadIndex = 0;

	// Fragments répertoriés
	TArray<FVoiceSnippet> StoredSnippets;

	// Verrou de section critique pour accès concurrent (Audio Thread / Game Thread)
	mutable FCriticalSection AudioBufferLock;
};
