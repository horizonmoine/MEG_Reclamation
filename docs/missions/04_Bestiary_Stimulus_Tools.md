# MISSION 04 — Brancher le bestiaire et les outils sur ULiminalStimulusSubsystem

Respecte `.agents/rules/ue5_cpp_network_rules.md` et `AGENTS.md` section `bestiary_ai_engineer`. Prérequis : Missions 01 à 03.

## Déjà livré (ne pas recréer)
- `AI/LiminalStimulusSubsystem.h/.cpp` : `ULiminalStimulusSubsystem` (`UWorldSubsystem`, serveur uniquement).
  - `ReportNoise(Location, LoudnessRms, Instigator)` : seuil RMS 0.03 (GDD), rayon = 60 m x intensité.
  - `ReportLight(Location, Intensity01, Instigator)` : rayon = 40 m x intensité.
  - `HasStimulusInRange(Type, Listener, MaxAge, Out)`, `GetRecentStimuli`, `ClearAll`, délégué natif `OnStimulusReported`.
- Test : `FMegStimulusSubsystemTest` (30 m entend, 90 m non).

## Contexte à lire
`AI/LiminalEntity.h/.cpp`, `AI/LiminalAIController.h/.cpp`, `AI/LiminalEntity_Hound.cpp`, `AI/LiminalEntity_Smiler.cpp`, `AI/LiminalEntityDirector.h/.cpp`, `AI/AnimNotify_LiminalAttackTrace.cpp`, `AI/VoiceMimicryComponent.h`, `Tools/BaseTool.h/.cpp`, `Tools/FlashStrobeTool.cpp`, `Tools/AudioDecoyTool.cpp`, `Player/LiminalFootstepComponent.cpp`.

## Reste à faire
1. `ALiminalEntity` : `bReplicates`, `SetReplicateMovement(true)`, `EEntityState` répliqué (`Idle, Hunting, Attacking, Stunned`) + `OnRep_EntityState` (anim/audio). Dégâts UNIQUEMENT dans `AnimNotify_LiminalAttackTrace` sous `HasAuthority()` via `PS->AuthApplyHealthDelta(-Damage)`.
2. `ALiminalAIController::BeginPlay` (serveur) : `Sub->OnStimulusReported.AddUObject(this, &...::HandleStimulus)`. Hound réagit à `Noise` à portée (Blackboard `TargetLocation`, `bIsHunting`), Smiler à `Light`. `RemoveAll(this)` dans `EndPlay`.
3. `ULiminalFootstepComponent` : côté serveur, chaque pas appelle `ReportNoise(Location, Loudness)` avec intensité fonction de vitesse/accroupissement (GDD : halètement 85 dB = 1.0, marche accroupie < 0.03).
4. `ABaseTool` : `UFUNCTION(Server, Reliable, WithValidation) ServerActivate()` sur l'owner pawn -> `OnServerActivated()` virtuel -> stimulus -> `UFUNCTION(NetMulticast, Unreliable) MulticastPlayEffect()`. `FlashStrobeTool` -> `ReportLight(1.0)`, `AudioDecoyTool` -> `ReportNoise(0.9)`. Les autres outils compilent sans changement.
5. `ALiminalEntityDirector` : spawn serveur uniquement ; en `Collapsing` (`GameState->GetMissionPhase()`) intensifie ; pacing par sanité moyenne des `ALiminalPlayerState`.
6. `VoiceMimicryComponent` (Skinwalker) : cible choisie serveur, répliquée par index dans `GameState->PlayerArray`, rendu vocal local.

## Assertions
- 0 error, 0 warning. Lint CI : 0 erreur.
- `grep -rn "AuthApplyHealthDelta" Source/MEG_Reclamation/AI/` : chaque occurrence est dans un bloc `HasAuthority()`.
- `FMegStimulusSubsystemTest` vert. `MegMeleeSafetyTests` étendu : un AttackTrace sans autorité ne modifie pas `Health`.
- `Run_Automation_Tests.ps1` : 100 %.
