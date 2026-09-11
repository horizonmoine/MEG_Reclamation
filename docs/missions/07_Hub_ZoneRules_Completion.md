# MISSION 07 — Hub Base Alpha jouable et cohérent avec les règles de zone

Respecte `.agents/rules/ue5_cpp_network_rules.md` et `docs/design/LEVEL_DESIGN_BIBLE.md` sections 1 et 2. Indépendante des missions 01 à 05 (peut être lancée en premier).

## Déjà livré
- `GameModes/LiminalZoneRulesSubsystem` : `IsMissionHostile`, `IsLocationInSafeZone`, `IsSanityPressureActiveFor`, `IsEntitySpawnAllowedAt`, `GetSafeZoneSanityRestoreRateFor`.
- `Objects/LiminalSafeZoneVolume` : volume plaçable, restauration de sanité, blocage des spawns.
- `AScavengerCharacter::UpdateSanityPressure` : garde en tête (plus de drain en Hub/lobby/sas).
- Test `FMegZoneRulesTest`.

## Contexte à lire
`GameModes/LiminalLobbyGameMode.h/.cpp`, `Objects/LiminalTerminalActor.h/.cpp`, `Objects/LiminalAirlockActor.h/.cpp`, `Hub/LiminalHubProgressionComponent.h`, `Data/LiminalGameInstance.h` (`TravelToMission`, `ReturnToHub`), `UI/LiminalDebriefHUD.h`, `Player/ScavengerCharacter.cpp` (`UpdateLocalEffects`, `SpawnHallucination`, `EmitFootstepNoise`).

## Reste à faire (code)
1. `AScavengerCharacter::UpdateLocalEffects` / `SpawnHallucination` : aucune hallucination si `!ULiminalZoneRulesSubsystem::IsSanityPressureActive(this)`. Hisser l'include ajouté au-dessus de `UpdateSanityPressure` en tête de fichier.
2. `ALiminalEntityDirector` et tout spawner : refuser un spawn si `!Rules->IsEntitySpawnAllowedAt(Location)`.
3. `ALiminalAirlockActor` : porte un `ALiminalSafeZoneVolume` enfant (extent = volume du sas) actif tant que `!bIsSealed`.
4. `ALiminalLobbyGameMode` (ou le GameMode du Hub) : utiliser `ALiminalGameState` avec `MissionPhase = Hub` et `ALiminalPlayerState`, pour que Hub et biomes partagent la même source de vérité. Aucun timer démarré en Hub.
5. `ULiminalGameInstance::ReturnToHub` : `PS->AuthResetForNewRun()` sur chaque PlayerState avant le travel ; `GS->AuthSetMissionPhase(Hub)`.
6. `ALiminalTerminalActor::ServerLaunchIncursion` : exige `bIsReady` sur tous les PlayerStates et tous `Alive` dans le volume du sas.

## Reste à faire (map `Lvl_Hub_BaseAlpha`, via MCP Unreal ou éditeur)
1. Placer un `ALiminalSafeZoneVolume` englobant toute la map (extent 2500 x 2000 x 500, `SanityRestorePerSecond = 2`).
2. Placer un second volume sur l'infirmerie (`SanityRestorePerSecond = 8`).
3. Vérifier la présence : 4 `PlayerStart`, 1 `ALiminalTerminalActor`, 1 `ALiminalAirlockActor`, tableau des biomes, `ALiminalFuseBoxActor` tutoriel, éclairage stable 4000 K sans `ULiminalFlickerLightComponent`.
4. Bloquer les sorties : aucune géométrie ouverte hors du sas (test : marcher 60 s le long des murs).

## Assertions
- Build 0 error / 0 warning. Lint CI 0 erreur.
- `FMegZoneRulesTest` vert + nouveau test : `SpawnHallucination` n'ajoute aucun `ALiminalHallucinationActor` quand la phase est `Hub`.
- Test manuel : 120 s seul dans le Hub, sanité reste à 100 %, aucune hallucination, aucun bruit d'entité.
- `Run_Auto_Check.ps1` : ajouter un contrôle « la map Hub contient au moins un `LiminalSafeZoneVolume` ».
