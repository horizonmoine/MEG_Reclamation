# MISSION 09 — Directeur d'entités et courbe de pacing 480 s

Respecte `.agents/rules/ue5_cpp_network_rules.md`, `AGENTS.md` section `bestiary_ai_engineer`, `docs/design/LEVEL_DESIGN_BIBLE.md` sections 3, 5, 6. Prérequis : Missions 02, 04, 07.

## Contexte à lire
`AI/LiminalEntityDirector.h/.cpp`, `AI/LiminalEntity.h`, `GameModes/LiminalGameState.h`, `GameModes/LiminalZoneRulesSubsystem.h`, `AI/LiminalStimulusSubsystem.h`, `Events/LiminalEventSubsystem.h`, `GameModes/LiminalGameMode.cpp` (`TriggerBlackout`).

## Reste à faire
1. `ALiminalEntityDirector` : serveur uniquement (`HasAuthority`), piloté par `FTimerHandle` 1 s (pas de `Tick`). Lit `GS->GetCollapseProgress()` et `GS->GetMissionPhase()`.
2. `USTRUCT FLiminalPacingWindow { float StartProgress; float EndProgress; int32 MaxActiveEntities; TArray<EMonsterType> AllowedTypes; float SpawnIntervalSeconds; }` ; tableau `UPROPERTY(EditDefaultsOnly)` par biome (`UDataAsset ULiminalBiomePacingProfile`) rempli d'après la table section 3 : Insertion 0-0.19, Tension 0.19-0.62, Escalade 0.62-0.80, Effondrement 0.80-1.0.
3. Plafond d'entités simultanées par N (2/3/4/5) et par fenêtre. Spawn uniquement si `Rules->IsEntitySpawnAllowedAt(Location)`, à ≥ 30 m du joueur le plus proche, hors ligne de vue.
4. Modulation par sanité moyenne des `ALiminalPlayerState` : < 40 % => prochain spawn +20 s ; > 80 % => -15 s.
5. `Collapsing` : stun des entités x0.5, spawns à 30 m, `TriggerBlackout` par zones, sirène via `ULiminalAudioSubsystem`.
6. Le Directeur s'abonne à `ULiminalStimulusSubsystem::OnStimulusReported` pour orienter le prochain spawn vers la zone la plus bruyante (biais, pas déterminisme).
7. Roster par biome d'après la table section 5 ; Jerry = événement rare (≤ 1 par run, biomes Level 5 uniquement).

## Assertions
- Build 0 error / 0 warning. Lint CI 0 erreur.
- `Tests/MegDirectorPacingTests.cpp` : à progression 0.1, `MaxActiveEntities == 0` ; à 0.5 avec N=2, `== 3` ; à 0.9, types autorisés incluent la horde ; sanité moyenne 30 % => intervalle +20 s ; spawn refusé dans une `ALiminalSafeZoneVolume`.
- Session de test 8 min avec 2 joueurs : aucune entité avant 90 s, extraction possible, `Failed` à 480 s si non extrait.
