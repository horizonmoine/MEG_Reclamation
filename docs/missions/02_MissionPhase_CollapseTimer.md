# MISSION 02 — Brancher le GameMode et le Sas sur la machine à états du GameState

Respecte `.agents/rules/ue5_cpp_network_rules.md`. Prérequis : Mission 01.

## Déjà livré (ne pas recréer)
- `ALiminalGameState` : `EMissionPhase` (`Hub, Airlock, Incursion, Collapsing, Extracted, Failed`), `AuthSetMissionPhase` avec `IsPhaseTransitionValid`, `OnMissionPhaseChanged`.
- Timer : `AuthStartCollapseTimer(Duration)`, `AuthStopCollapseTimer`, `GetCollapseTimeRemaining()` (dérivé de `GetServerWorldTimeSeconds()`), `GetCollapseProgress()`, `IsCollapseTimerRunning()`. `GetStabilityPhase()` dérivé du timer quand il tourne.
- `MissionSeed` répliqué (`AuthSetMissionSeed`), `TeamBankCredits` (`AuthAddTeamBankCredits`, `OnTeamBankCreditsChanged`).
- `ALiminalGameMode::BeginPlay` : génère une seed unique, l'écrit dans le GameState, passe en `Incursion`, démarre le timer avec `TotalMissionDuration`, et la passe au `ALiminalLevelGenerator`.
- Test : `FMegMissionPhaseTimerTest`.

## Contexte à lire
`GameModes/LiminalGameMode.cpp` (390 lignes : `Tick`, `CheckGameOverCondition`, `HandleSquadWiped`, `HandleMissionSuccess`, `TriggerExtraction`), `Objects/LiminalAirlockActor.h/.cpp`, `Objects/ExtractionZone.h/.cpp`, `UI/LiminalScavengerHUD.cpp`, `UI/ScavengerHUDWidget.cpp`.

## Reste à faire
1. `ALiminalGameMode` : supprimer la décrémentation de `RealityCollapseTimer` dans `Tick`. Remplacer par `FTimerHandle CollapseTimerHandle` armé dans `StartIncursion()` -> `OnCollapseTimerExpired()` -> `FinishMission(false)`. `GetRealityCollapseRemainingSeconds()` délègue à `GameState->GetCollapseTimeRemaining()`.
2. Mapper l'ancien `EExtractionMatchState` sur `EMissionPhase` : `HandleMissionSuccess` -> `AuthSetMissionPhase(Extracted)`, `HandleSquadWiped` -> `Failed`. Conserver `GetMatchState()` comme façade dérivée pour compatibilité.
3. Phase `Collapsing` : déclenchée par le GameMode quand `GetCollapseProgress() >= 0.8` (vérifié par un `FTimerHandle` périodique 1 s, pas dans `Tick`).
4. `ALiminalAirlockActor` : `bIsSealed` répliqué + `OnRep_IsSealed` (portes, audio). `AScavengerCharacter::ServerRequestSealAirlock(ALiminalAirlockActor*)` (Reliable, WithValidation, distance < 300 cm). Le serveur vérifie que tous les PlayerStates `Alive` sont dans le volume, puis GameMode `StartIncursion()`. Déplacer le démarrage du timer de `BeginPlay` vers `StartIncursion()` pour les maps avec sas ; conserver le démarrage immédiat si aucun sas dans la map.
5. `AExtractionZone` : serveur -> `PS->AuthSetStatus(Extracted)`, `GameState->AuthAddTeamBankCredits(PS->GetCarriedCredits())`, `PS->AuthSetCarriedCredits(0)`. `FinishMission(true)` quand tous les non-`Dead` sont `Extracted`.
6. HUD : le compte à rebours lit `GameState->GetCollapseTimeRemaining()` (fonctionne sur clients et late-join), plus jamais le GameMode.

## Assertions
- 0 error, 0 warning en Development ET Shipping Win64.
- `grep -n "RealityCollapseTimer -=" Source/` ne retourne rien.
- Lint CI : 0 erreur. `FMegMissionPhaseTimerTest` vert.
- Nouveau test `Tests/MegMissionFlowTests.cpp` : un GameMode de test appelle `StartIncursion`, puis `FinishMission(false)` => phase `Failed` ; `FinishMission(true)` depuis `Incursion` => `Extracted`.
- `Run_Auto_Check.ps1` : tous verts.
