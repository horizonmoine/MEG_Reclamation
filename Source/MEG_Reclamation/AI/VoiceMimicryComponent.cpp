#include "AI/VoiceMimicryComponent.h"
#include "Misc/ScopeLock.h"
#include "Engine/World.h"

UVoiceMimicryComponent::UVoiceMimicryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Pre-allocation in constructor to ensure no dynamic memory allocation occurs in the audio thread
	CircularSampleBuffer.SetNumZeroed(MaxBufferSamples);
	StoredSnippets.Reserve(MaxStoredSnippets);
}

void UVoiceMimicryComponent::BeginPlay()
{
	Super::BeginPlay();

	FScopeLock Lock(&AudioBufferLock);
	WriteHeadIndex = 0;
	StoredSnippets.Reset();
}

bool UVoiceMimicryComponent::PushAudioSamples(const float* InSamples, int32 NumSamples, FName SpeakerId)
{
	if (!InSamples || NumSamples <= 0)
	{
		return false;
	}

	FScopeLock Lock(&AudioBufferLock);

	const int32 StartIndex = WriteHeadIndex;
	const int32 SafeSamples = FMath::Min(NumSamples, MaxBufferSamples);

	for (int32 i = 0; i < SafeSamples; ++i)
	{
		CircularSampleBuffer[WriteHeadIndex] = InSamples[i];
		WriteHeadIndex = (WriteHeadIndex + 1) % MaxBufferSamples;
	}

	FVoiceSnippet Snippet;
	Snippet.SpeakerId = SpeakerId.IsNone() ? FName("AlliedScavenger") : SpeakerId;
	Snippet.DurationSeconds = static_cast<float>(SafeSamples) / 16000.0f;
	Snippet.Timestamp = (GetWorld()) ? GetWorld()->GetTimeSeconds() : 0.0f;
	Snippet.SampleCount = SafeSamples;
	Snippet.BufferStartIndex = StartIndex;

	if (StoredSnippets.Num() >= MaxStoredSnippets)
	{
		StoredSnippets.RemoveAt(0);
	}
	StoredSnippets.Add(Snippet);

	OnVoiceSnippetRecorded.Broadcast(Snippet);
	return true;
}

bool UVoiceMimicryComponent::GetLatestSnippet(FVoiceSnippet& OutSnippet) const
{
	FScopeLock Lock(&AudioBufferLock);
	if (StoredSnippets.IsEmpty())
	{
		return false;
	}
	OutSnippet = StoredSnippets.Last();
	return true;
}

void UVoiceMimicryComponent::SimulateCaptureVoiceSnippet(FName SpeakerName, float DurationSeconds)
{
	FScopeLock Lock(&AudioBufferLock);

	const int32 NumSamples = FMath::Clamp(FMath::RoundToInt(DurationSeconds * 16000.0f), 1600, MaxBufferSamples / 2);
	const int32 StartIndex = WriteHeadIndex;

	// Fill simulated synthetic audio in pre-allocated circular buffer (sine wave tone)
	for (int32 i = 0; i < NumSamples; ++i)
	{
		CircularSampleBuffer[WriteHeadIndex] = FMath::Sin(static_cast<float>(i) * 0.05f) * 0.5f;
		WriteHeadIndex = (WriteHeadIndex + 1) % MaxBufferSamples;
	}

	FVoiceSnippet Snippet;
	Snippet.SpeakerId = SpeakerName.IsNone() ? FName("Operative_Bravo") : SpeakerName;
	Snippet.DurationSeconds = DurationSeconds;
	Snippet.Timestamp = (GetWorld()) ? GetWorld()->GetTimeSeconds() : 0.0f;
	Snippet.SampleCount = NumSamples;
	Snippet.BufferStartIndex = StartIndex;

	if (StoredSnippets.Num() >= MaxStoredSnippets)
	{
		StoredSnippets.RemoveAt(0);
	}
	StoredSnippets.Add(Snippet);

	OnVoiceSnippetRecorded.Broadcast(Snippet);
}

bool UVoiceMimicryComponent::TriggerMimicryCall(const FVector& EmissionLocation, FVoiceSnippet& OutSnippetUsed)
{
	FScopeLock Lock(&AudioBufferLock);

	if (StoredSnippets.IsEmpty())
	{
		// If empty, generate a distress call simulation
		SimulateCaptureVoiceSnippet(FName("TrappedOperative"), 1.8f);
	}

	if (!StoredSnippets.IsEmpty())
	{
		const int32 RandomIndex = FMath::RandRange(0, StoredSnippets.Num() - 1);
		OutSnippetUsed = StoredSnippets[RandomIndex];

		UE_LOG(LogTemp, Log, TEXT("[VoiceMimicry] Skinwalker mimicked voice of '%s' (duration %.1fs) at %s"),
			*OutSnippetUsed.SpeakerId.ToString(), OutSnippetUsed.DurationSeconds, *EmissionLocation.ToString());

		OnVoiceMimicked.Broadcast(OutSnippetUsed.SpeakerId, EmissionLocation);
		return true;
	}

	return false;
}

int32 UVoiceMimicryComponent::GetStoredSnippetCount() const
{
	FScopeLock Lock(&AudioBufferLock);
	return StoredSnippets.Num();
}

bool UVoiceMimicryComponent::HasAnyRecordedVoices() const
{
	FScopeLock Lock(&AudioBufferLock);
	return !StoredSnippets.IsEmpty();
}
