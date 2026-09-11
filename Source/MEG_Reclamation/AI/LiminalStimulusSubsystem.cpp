#include "AI/LiminalStimulusSubsystem.h"

#include "Engine/World.h"

bool FLiminalStimulus::IsInRange(const FVector& ListenerLocation) const
{
	return FVector::DistSquared(Location, ListenerLocation) <= FMath::Square(RadiusCm);
}

bool ULiminalStimulusSubsystem::ReportNoise(const FVector& Location, float LoudnessRms, AActor* Instigator)
{
	if (!IsServerAuthority())
	{
		return false;
	}

	const float Loudness = FMath::Clamp(LoudnessRms, 0.0f, 1.0f);
	if (Loudness < NoiseRmsThreshold)
	{
		return false;
	}

	FLiminalStimulus Stimulus;
	Stimulus.Type = ELiminalStimulusType::Noise;
	Stimulus.Location = Location;
	Stimulus.Strength = Loudness;
	Stimulus.RadiusCm = MaxNoiseRadiusCm * Loudness;
	Stimulus.ServerTimeSeconds = GetNowSeconds();
	Stimulus.Instigator = Instigator;
	return Push(MoveTemp(Stimulus));
}

bool ULiminalStimulusSubsystem::ReportLight(const FVector& Location, float Intensity01, AActor* Instigator)
{
	if (!IsServerAuthority())
	{
		return false;
	}

	const float Intensity = FMath::Clamp(Intensity01, 0.0f, 1.0f);
	if (Intensity <= 0.0f)
	{
		return false;
	}

	FLiminalStimulus Stimulus;
	Stimulus.Type = ELiminalStimulusType::Light;
	Stimulus.Location = Location;
	Stimulus.Strength = Intensity;
	Stimulus.RadiusCm = MaxLightRadiusCm * Intensity;
	Stimulus.ServerTimeSeconds = GetNowSeconds();
	Stimulus.Instigator = Instigator;
	return Push(MoveTemp(Stimulus));
}

bool ULiminalStimulusSubsystem::HasStimulusInRange(ELiminalStimulusType Type, const FVector& ListenerLocation, float MaxAgeSeconds, FLiminalStimulus& OutStrongest) const
{
	const float Now = GetNowSeconds();
	bool bFound = false;

	for (const FLiminalStimulus& Stimulus : Stimuli)
	{
		if (Stimulus.Type != Type)
		{
			continue;
		}
		if (Now - Stimulus.ServerTimeSeconds > MaxAgeSeconds)
		{
			continue;
		}
		if (!Stimulus.IsInRange(ListenerLocation))
		{
			continue;
		}
		if (!bFound || Stimulus.Strength > OutStrongest.Strength)
		{
			OutStrongest = Stimulus;
			bFound = true;
		}
	}

	return bFound;
}

int32 ULiminalStimulusSubsystem::GetRecentStimuli(ELiminalStimulusType Type, float MaxAgeSeconds, TArray<FLiminalStimulus>& OutStimuli) const
{
	OutStimuli.Reset();
	const float Now = GetNowSeconds();

	for (const FLiminalStimulus& Stimulus : Stimuli)
	{
		if (Stimulus.Type == Type && Now - Stimulus.ServerTimeSeconds <= MaxAgeSeconds)
		{
			OutStimuli.Add(Stimulus);
		}
	}

	return OutStimuli.Num();
}

void ULiminalStimulusSubsystem::ClearAll()
{
	if (!IsServerAuthority())
	{
		return;
	}

	Stimuli.Reset();
}

bool ULiminalStimulusSubsystem::IsServerAuthority() const
{
	const UWorld* World = GetWorld();
	return World && World->GetNetMode() != NM_Client;
}

float ULiminalStimulusSubsystem::GetNowSeconds() const
{
	const UWorld* World = GetWorld();
	return World ? static_cast<float>(World->GetTimeSeconds()) : 0.0f;
}

void ULiminalStimulusSubsystem::Prune(float NowSeconds)
{
	Stimuli.RemoveAll([NowSeconds](const FLiminalStimulus& Stimulus)
	{
		return NowSeconds - Stimulus.ServerTimeSeconds > StimulusLifetimeSeconds;
	});
}

bool ULiminalStimulusSubsystem::Push(FLiminalStimulus&& Stimulus)
{
	Prune(Stimulus.ServerTimeSeconds);

	if (Stimuli.Num() >= MaxStoredStimuli)
	{
		Stimuli.RemoveAt(0);
	}

	const FLiminalStimulus& Stored = Stimuli.Add_GetRef(MoveTemp(Stimulus));
	OnStimulusReported.Broadcast(Stored);
	return true;
}
