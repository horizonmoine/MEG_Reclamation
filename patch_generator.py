import re
import os

cpp_file = "F:/MEG_Reclamation/Source/MEG_Reclamation/ProcGen/LiminalLevelGenerator.cpp"

with open(cpp_file, 'r', encoding='utf-8') as f:
    content = f.read()

# Add includes
if '#include "ProcGen/LiminalFlickerLightComponent.h"' not in content:
    inc = """#include "ProcGen/LiminalFlickerLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
"""
    content = content.replace('#include "Audio/LiminalAudioSubsystem.h"', '#include "Audio/LiminalAudioSubsystem.h"\n' + inc)

# Add Fog and PPVolume at the end of BuildVisuals (before ClearSpawnedActors maybe)
# Let's find: `for (TActorIterator<ASkyLight> SkyIt(World); SkyIt; ++SkyIt)`
# Actually, let's find `if (bGameWorld)` inside BuildVisuals, and at the end of BuildVisuals add the Fog and PPVolume.

fog_pp_code = """
		if (bGameWorld)
		{
			AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
			if (Fog && Fog->GetComponent())
			{
				Fog->GetComponent()->bEnableVolumetricFog = true;
				float FogDensity = 0.02f;
				if (Biome == ELevelBiome::Level0_YellowLobby || Biome == ELevelBiome::Level8_CaveSystem) FogDensity = 0.1f;
				else if (Biome == ELevelBiome::Level37_Poolrooms || Biome == ELevelBiome::Level4_AbandonedOffice) FogDensity = 0.005f;
				Fog->GetComponent()->SetFogDensity(FogDensity);
				CosmeticActors.Add(Fog);
			}

			APostProcessVolume* PPVolume = World->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
			if (PPVolume)
			{
				PPVolume->bUnbound = true;
				FPostProcessSettings& Settings = PPVolume->Settings;
				if (Biome == ELevelBiome::Level0_YellowLobby)
				{
					Settings.bOverride_ColorSaturation = true;
					Settings.ColorSaturation = FVector4(1.2f, 1.1f, 0.9f, 1.0f);
					Settings.bOverride_FilmGrainIntensity = true;
					Settings.FilmGrainIntensity = 0.5f;
					Settings.bOverride_VignetteIntensity = true;
					Settings.VignetteIntensity = 0.4f;
				}
				else if (Biome == ELevelBiome::Level8_CaveSystem)
				{
					Settings.bOverride_ColorContrast = true;
					Settings.ColorContrast = FVector4(1.3f, 1.3f, 1.3f, 1.0f);
					Settings.bOverride_VignetteIntensity = true;
					Settings.VignetteIntensity = 0.8f;
				}
				else if (Biome == ELevelBiome::Level37_Poolrooms)
				{
					Settings.bOverride_ColorSaturation = true;
					Settings.ColorSaturation = FVector4(0.8f, 0.9f, 1.2f, 1.0f);
					Settings.bOverride_BloomIntensity = true;
					Settings.BloomIntensity = 1.5f;
					Settings.bOverride_SceneFringeIntensity = true;
					Settings.SceneFringeIntensity = 2.0f;
				}
				else if (Biome == ELevelBiome::LevelRun_RunForYourLife || Biome == ELevelBiome::Level4_AbandonedOffice)
				{
					Settings.bOverride_ColorSaturation = true;
					Settings.ColorSaturation = FVector4(0.6f, 0.6f, 0.6f, 1.0f);
					Settings.bOverride_ColorContrast = true;
					Settings.ColorContrast = FVector4(1.4f, 1.4f, 1.4f, 1.0f);
					Settings.bOverride_MotionBlurAmount = true;
					Settings.MotionBlurAmount = 0.8f;
				}
				CosmeticActors.Add(PPVolume);
			}
		}
"""

if 'AExponentialHeightFog*' not in content:
    content = content.replace('void ALiminalLevelGenerator::ClearSpawnedActors()', fog_pp_code + '\nvoid ALiminalLevelGenerator::ClearSpawnedActors()')

flicker_code = """
		if (FMath::FRand() < 0.3f)
		{
			ULiminalFlickerLightComponent* Flicker = NewObject<ULiminalFlickerLightComponent>(Lamp);
			Flicker->RegisterComponent();
		}
		CosmeticActors.Add(Lamp);"""

if 'ULiminalFlickerLightComponent*' not in content:
    content = content.replace('CosmeticActors.Add(Lamp);', flicker_code)

with open(cpp_file, 'w', encoding='utf-8') as f:
    f.write(content)

print("Done replacing.")
