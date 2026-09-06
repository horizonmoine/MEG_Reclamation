#include "Audio/LiminalAudioSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"
#include "GameFramework/PlayerController.h"

ULiminalAudioSubsystem::ULiminalAudioSubsystem()
{
	BuildDefaultProfiles();
}

void ULiminalAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	SetCurrentBiome(ELevelBiome::Level0_YellowLobby);
	ScheduleNextStinger();
}

void ULiminalAudioSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StingerTimerHandle);
	}
	Super::Deinitialize();
}

void ULiminalAudioSubsystem::BuildDefaultProfiles()
{
	BiomeProfiles.Empty();

	// 1. Level0_YellowLobby (60Hz hum, medium reverberation)
	{
		FLiminalBiomeAudioProfile P;
		P.Biome = ELevelBiome::Level0_YellowLobby;
		P.AmbientLoopId = FName("Amb_YellowLobby_60HzHum");
		P.AmbientVolume = 0.85f;
		P.DronePitch = 1.0f;
		P.ReverbDecaySeconds = 2.8f;
		P.ReverbDamping = 0.4f;
		P.LowPassCutoffHz = 20000.0f;
		P.StingerIntervalMin = 8.0f;
		P.StingerIntervalMax = 18.0f;
		P.StingerIds = { FName("Stinger_FluorescentFlicker"), FName("Stinger_DistantCarpetThud"), FName("Stinger_ElectricalPop") };
		BiomeProfiles.Add(P.Biome, P);
	}

	// 2. Level1_HabitableZone (Cold concrete echo, distant machinery)
	{
		FLiminalBiomeAudioProfile P;
		P.Biome = ELevelBiome::Level1_HabitableZone;
		P.AmbientLoopId = FName("Amb_HabitableZone_ConcreteWind");
		P.AmbientVolume = 0.75f;
		P.DronePitch = 0.95f;
		P.ReverbDecaySeconds = 3.5f;
		P.ReverbDamping = 0.3f;
		P.LowPassCutoffHz = 18000.0f;
		P.StingerIntervalMin = 10.0f;
		P.StingerIntervalMax = 22.0f;
		P.StingerIds = { FName("Stinger_VentilationCreak"), FName("Stinger_PuddleSplash") };
		BiomeProfiles.Add(P.Biome, P);
	}

	// 3. Level2_PipeDreams (Hissing steam, dripping, tight metal reverb)
	{
		FLiminalBiomeAudioProfile P;
		P.Biome = ELevelBiome::Level2_PipeDreams;
		P.AmbientLoopId = FName("Amb_PipeDreams_SteamPipes");
		P.AmbientVolume = 0.9f;
		P.DronePitch = 1.05f;
		P.ReverbDecaySeconds = 1.4f;
		P.ReverbDamping = 0.65f;
		P.LowPassCutoffHz = 16000.0f;
		P.StingerIntervalMin = 6.0f;
		P.StingerIntervalMax = 15.0f;
		P.StingerIds = { FName("Stinger_PipeClank"), FName("Stinger_SteamBurst"), FName("Stinger_PressureValveRelease") };
		BiomeProfiles.Add(P.Biome, P);
	}

	// 4. Level3_ElectricalStation (High-voltage crackle and sub-bass drone)
	{
		FLiminalBiomeAudioProfile P;
		P.Biome = ELevelBiome::Level3_ElectricalStation;
		P.AmbientLoopId = FName("Amb_ElectricalStation_HighVoltage");
		P.AmbientVolume = 0.95f;
		P.DronePitch = 1.15f;
		P.ReverbDecaySeconds = 2.2f;
		P.ReverbDamping = 0.5f;
		P.LowPassCutoffHz = 22000.0f;
		P.StingerIntervalMin = 5.0f;
		P.StingerIntervalMax = 14.0f;
		P.StingerIds = { FName("Stinger_ArcDischarge"), FName("Stinger_TransformerSurge") };
		BiomeProfiles.Add(P.Biome, P);
	}

	// 5. Level4_AbandonedOffice (Eerie silence, ticking clocks, distant fax tone)
	{
		FLiminalBiomeAudioProfile P;
		P.Biome = ELevelBiome::Level4_AbandonedOffice;
		P.AmbientLoopId = FName("Amb_Office_EmptyHalls");
		P.AmbientVolume = 0.65f;
		P.DronePitch = 0.9f;
		P.ReverbDecaySeconds = 3.2f;
		P.ReverbDamping = 0.35f;
		P.LowPassCutoffHz = 19000.0f;
		P.StingerIntervalMin = 12.0f;
		P.StingerIntervalMax = 25.0f;
		P.StingerIds = { FName("Stinger_KeyboardClick"), FName("Stinger_DistantTelephoneRing"), FName("Stinger_CeilingTileFall") };
		BiomeProfiles.Add(P.Biome, P);
	}

	// 6. Level6_LightsOut (Near total silence, subtle breathing, heartbeat)
	{
		FLiminalBiomeAudioProfile P;
		P.Biome = ELevelBiome::Level6_LightsOut;
		P.AmbientLoopId = FName("Amb_LightsOut_VoidPulse");
		P.AmbientVolume = 0.5f;
		P.DronePitch = 0.7f;
		P.ReverbDecaySeconds = 4.5f;
		P.ReverbDamping = 0.2f;
		P.LowPassCutoffHz = 8000.0f;
		P.StingerIntervalMin = 7.0f;
		P.StingerIntervalMax = 16.0f;
		P.StingerIds = { FName("Stinger_SubtleWhisper"), FName("Stinger_HeartbeatThud"), FName("Stinger_ScrapingClaw") };
		BiomeProfiles.Add(P.Biome, P);
	}

	// 7. Level8_CaveSystem (Karst drip, deep earth rumble, cavernous resonance)
	{
		FLiminalBiomeAudioProfile P;
		P.Biome = ELevelBiome::Level8_CaveSystem;
		P.AmbientLoopId = FName("Amb_CaveSystem_DeepEchoes");
		P.AmbientVolume = 0.8f;
		P.DronePitch = 0.85f;
		P.ReverbDecaySeconds = 5.0f;
		P.ReverbDamping = 0.25f;
		P.LowPassCutoffHz = 14000.0f;
		P.StingerIntervalMin = 8.0f;
		P.StingerIntervalMax = 20.0f;
		P.StingerIds = { FName("Stinger_RockSlide"), FName("Stinger_WaterDropEcho"), FName("Stinger_ArachnidSkitter") };
		BiomeProfiles.Add(P.Biome, P);
	}

	// 8. Level9_DarkSuburbs (Wind howling through power lines, distant siren)
	{
		FLiminalBiomeAudioProfile P;
		P.Biome = ELevelBiome::Level9_DarkSuburbs;
		P.AmbientLoopId = FName("Amb_DarkSuburbs_NightWind");
		P.AmbientVolume = 0.8f;
		P.DronePitch = 0.95f;
		P.ReverbDecaySeconds = 2.0f;
		P.ReverbDamping = 0.5f;
		P.LowPassCutoffHz = 17000.0f;
		P.StingerIntervalMin = 10.0f;
		P.StingerIntervalMax = 24.0f;
		P.StingerIds = { FName("Stinger_DistantCarHorn"), FName("Stinger_DoorSlamFarAway"), FName("Stinger_WindChime") };
		BiomeProfiles.Add(P.Biome, P);
	}

	// 9. Level10_WheatFields (Rustling stalks, insect drones, twilight breeze)
	{
		FLiminalBiomeAudioProfile P;
		P.Biome = ELevelBiome::Level10_WheatFields;
		P.AmbientLoopId = FName("Amb_WheatFields_GrassChirp");
		P.AmbientVolume = 0.7f;
		P.DronePitch = 1.0f;
		P.ReverbDecaySeconds = 1.2f;
		P.ReverbDamping = 0.8f;
		P.LowPassCutoffHz = 21000.0f;
		P.StingerIntervalMin = 12.0f;
		P.StingerIntervalMax = 30.0f;
		P.StingerIds = { FName("Stinger_GrassMovementFast"), FName("Stinger_CicadaSwell") };
		BiomeProfiles.Add(P.Biome, P);
	}

	// 10. Level37_Poolrooms (Tiled acoustics, echoing lapping water, subaquatic dampening)
	{
		FLiminalBiomeAudioProfile P;
		P.Biome = ELevelBiome::Level37_Poolrooms;
		P.AmbientLoopId = FName("Amb_Poolrooms_LappingWater");
		P.AmbientVolume = 0.85f;
		P.DronePitch = 0.9f;
		P.ReverbDecaySeconds = 4.2f;
		P.ReverbDamping = 0.15f;
		P.LowPassCutoffHz = 12000.0f;
		P.StingerIntervalMin = 9.0f;
		P.StingerIntervalMax = 19.0f;
		P.StingerIds = { FName("Stinger_TileEchoFootstep"), FName("Stinger_DistantWaterSurge"), FName("Stinger_FilterHum") };
		BiomeProfiles.Add(P.Biome, P);
	}

	// 11. LevelRun_RunForYourLife (Loud blaring alarm siren, pounding techno-industrial drone)
	{
		FLiminalBiomeAudioProfile P;
		P.Biome = ELevelBiome::LevelRun_RunForYourLife;
		P.AmbientLoopId = FName("Amb_RunForYourLife_AlarmSiren");
		P.AmbientVolume = 1.0f;
		P.DronePitch = 1.2f;
		P.ReverbDecaySeconds = 1.8f;
		P.ReverbDamping = 0.5f;
		P.LowPassCutoffHz = 22000.0f;
		P.StingerIntervalMin = 3.0f;
		P.StingerIntervalMax = 8.0f;
		P.StingerIds = { FName("Stinger_SirenSweep"), FName("Stinger_EmergencyKlaxon"), FName("Stinger_MetalDoorBanging") };
		BiomeProfiles.Add(P.Biome, P);
	}
}

void ULiminalAudioSubsystem::SetCurrentBiome(ELevelBiome NewBiome)
{
	CurrentBiome = NewBiome;
	if (const FLiminalBiomeAudioProfile* Found = BiomeProfiles.Find(NewBiome))
	{
		ActiveProfile = *Found;
	}
	else
	{
		ActiveProfile = FLiminalBiomeAudioProfile();
		ActiveProfile.Biome = NewBiome;
	}

	DynamicReverbDecay = ActiveProfile.ReverbDecaySeconds;
	DynamicLowPassCutoff = ActiveProfile.LowPassCutoffHz;

	OnBiomeAudioChanged.Broadcast(CurrentBiome, ActiveProfile);
	ScheduleNextStinger();
}

FLiminalBiomeAudioProfile ULiminalAudioSubsystem::GetProfileForBiome(ELevelBiome Biome) const
{
	if (const FLiminalBiomeAudioProfile* Found = BiomeProfiles.Find(Biome))
	{
		return *Found;
	}
	FLiminalBiomeAudioProfile Fallback;
	Fallback.Biome = Biome;
	return Fallback;
}

void ULiminalAudioSubsystem::UpdateRoomAcoustics(float RoomVolumeMetersCubed, float WallHardnessFactor)
{
	const float ClampedVolume = FMath::Clamp(RoomVolumeMetersCubed, 20.0f, 5000.0f);
	const float VolumeScale = FMath::Loge(ClampedVolume / 20.0f) / FMath::Loge(250.0f);
	const float ClampedHardness = FMath::Clamp(WallHardnessFactor, 0.1f, 1.0f);

	DynamicReverbDecay = FMath::Clamp(ActiveProfile.ReverbDecaySeconds * (0.6f + 0.8f * VolumeScale) * ClampedHardness, 0.3f, 8.0f);
	DynamicLowPassCutoff = FMath::Clamp(ActiveProfile.LowPassCutoffHz * (0.7f + 0.3f * ClampedHardness), 500.0f, 22000.0f);
}

void ULiminalAudioSubsystem::ScheduleNextStinger()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Interval = FMath::FRandRange(
		FMath::Max(1.0f, ActiveProfile.StingerIntervalMin),
		FMath::Max(2.0f, ActiveProfile.StingerIntervalMax));

	World->GetTimerManager().SetTimer(StingerTimerHandle, this, &ULiminalAudioSubsystem::OnStingerTimerExpired, Interval, false);
}

void ULiminalAudioSubsystem::OnStingerTimerExpired()
{
	if (ActiveProfile.StingerIds.Num() > 0)
	{
		// Procedural location within proximity
		TriggerProceduralStinger(FVector::ZeroVector);
	}
	ScheduleNextStinger();
}

void ULiminalAudioSubsystem::TriggerProceduralStinger(const FVector& Location)
{
	if (ActiveProfile.StingerIds.IsEmpty())
	{
		return;
	}

	const int32 Index = FMath::RandRange(0, ActiveProfile.StingerIds.Num() - 1);
	const FName StingerId = ActiveProfile.StingerIds[Index];

	FVector TargetLoc = Location;
	if (TargetLoc.IsZero())
	{
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				if (APawn* PlayerPawn = PC->GetPawn())
				{
					const float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
					const float Dist = FMath::FRandRange(600.0f, 1600.0f);
					TargetLoc = PlayerPawn->GetActorLocation() + FVector(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 20.0f);
				}
			}
		}
	}

	if (!TargetLoc.IsZero())
	{
		UAISense_Hearing::ReportNoiseEvent(GetWorld(), TargetLoc, 0.45f, nullptr);
	}

	UE_LOG(LogTemp, Verbose, TEXT("[LiminalAudio] Triggered procedural stinger '%s' at %s for biome '%d'"),
		*StingerId.ToString(), *TargetLoc.ToString(), static_cast<int32>(CurrentBiome));
}

void ULiminalAudioSubsystem::ProcessVoiceNoise(AActor* Speaker, float InputDecibels)
{
	if (!Speaker)
	{
		return;
	}

	// Decibels typically range from -60 dB (quiet) to 0 dB (screaming)
	// Map to loudness 0.0f to 1.5f for MakeNoise
	if (InputDecibels > -40.0f)
	{
		const float Normalized = FMath::Clamp((InputDecibels + 40.0f) / 40.0f, 0.1f, 1.5f);
		Speaker->MakeNoise(Normalized, Cast<APawn>(Speaker), Speaker->GetActorLocation());
	}
}
