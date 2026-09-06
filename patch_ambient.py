import os

cpp_file = "F:/MEG_Reclamation/Source/MEG_Reclamation/ProcGen/LiminalLevelGenerator.cpp"

with open(cpp_file, 'r', encoding='utf-8') as f:
    content = f.read()

# I will append audio play logic to `void ALiminalLevelGenerator::Generate(int32 InSeed)`
# specifically at the end of the method.

audio_code = """
	if (bGameWorld)
	{
		FString SoundPath = TEXT("");
		switch(Biome) {
			case ELevelBiome::Level0_YellowLobby: SoundPath = TEXT("/Game/Audio/S_Ambient_Lobby.S_Ambient_Lobby"); break;
			case ELevelBiome::Level1_HabitableZone: SoundPath = TEXT("/Game/Audio/S_Ambient_HabitableZone.S_Ambient_HabitableZone"); break;
			case ELevelBiome::Level2_PipeDreams: SoundPath = TEXT("/Game/Audio/S_Ambient_PipeDreams.S_Ambient_PipeDreams"); break;
			case ELevelBiome::Level3_ElectricalStation: SoundPath = TEXT("/Game/Audio/S_Ambient_Electrical.S_Ambient_Electrical"); break;
			case ELevelBiome::Level4_AbandonedOffice: SoundPath = TEXT("/Game/Audio/S_Ambient_Office.S_Ambient_Office"); break;
			case ELevelBiome::Level8_CaveSystem: SoundPath = TEXT("/Game/Audio/S_Ambient_Cave.S_Ambient_Cave"); break;
			case ELevelBiome::Level9_DarkSuburbs: SoundPath = TEXT("/Game/Audio/S_Ambient_Suburbs.S_Ambient_Suburbs"); break;
			case ELevelBiome::Level10_WheatFields: SoundPath = TEXT("/Game/Audio/S_Ambient_WheatFields.S_Ambient_WheatFields"); break;
			case ELevelBiome::Level37_Poolrooms: SoundPath = TEXT("/Game/Audio/S_Ambient_Poolrooms.S_Ambient_Poolrooms"); break;
			case ELevelBiome::LevelRun_RunForYourLife: SoundPath = TEXT("/Game/Audio/S_Ambient_Run.S_Ambient_Run"); break;
			default: break;
		}

		if (!SoundPath.IsEmpty())
		{
			USoundBase* AmbientSound = Cast<USoundBase>(StaticLoadObject(USoundBase::StaticClass(), nullptr, *SoundPath));
			if (AmbientSound)
			{
				UGameplayStatics::SpawnSound2D(this, AmbientSound);
			}
		}
	}
"""

if "SpawnSound2D(this, AmbientSound);" not in content:
    content = content.replace("AudioSub->UpdateRoomAcoustics(AvgVolume, 0.8f);\n\t\t\t}\n\t\t}\n\t}", "AudioSub->UpdateRoomAcoustics(AvgVolume, 0.8f);\n\t\t\t}\n\t\t}\n\t}\n" + audio_code)

with open(cpp_file, 'w', encoding='utf-8') as f:
    f.write(content)

print("Audio patched")
