# MISSION 01 — Migration du Character vers ALiminalPlayerState

Respecte `.agents/rules/ue5_cpp_network_rules.md`. Lis `docs/architecture/NETWORK_ARCHITECTURE.md`.

## Déjà livré (ne pas recréer)
- `Source/MEG_Reclamation/Player/LiminalPlayerState.h/.cpp` : `ALiminalPlayerState` complet.
  - Propriétés répliquées : `Sanity`, `Health`, `Status` (`EScavengerStatus`), `CarriedCredits`, `bIsInfectedPartygoer`, `bIsReady`.
  - Mutations serveur : `AuthApplySanityDelta`, `AuthApplyHealthDelta`, `AuthSetStatus` (table de transitions), `AuthAddCarriedCredits`, `AuthSetCarriedCredits`, `AuthSetInfectedPartygoer`, `AuthSetReady`, `AuthRevive`, `AuthResetForNewRun`.
  - Délégués : `OnSanityChanged(Old, New)`, `OnHealthChanged`, `OnStatusChanged`, `OnCarriedCreditsChanged`.
  - `CopyProperties` / `OverrideWith` pour SeamlessTravel.
- `ALiminalGameMode` : `PlayerStateClass = ALiminalPlayerState::StaticClass()`.
- Test : `Tests/MegNetworkFoundationTests.cpp` (`FMegPlayerStateAuthorityTest`).

## Contexte à lire
`Player/ScavengerCharacter.h/.cpp` (640 lignes de header), `Sanity/LiminalSanityPostProcessComponent.h/.cpp`, `Sanity/LiminalHallucinationActor.h`, `UI/ScavengerHUDWidget.h/.cpp`, `Player/LiminalSpectatorPawn.h`.

## Reste à faire
1. `AScavengerCharacter` : supprimer les champs membres `Sanity`, `Health`/`CurrentHealth`, `bIsDead`, `bIsDowned`, `bIsInfectedPartygoer`, `CarriedCredits` (et leurs `DOREPLIFETIME`). Ajouter un accesseur privé `ALiminalPlayerState* GetLiminalPlayerState() const`.
2. Réécrire en délégation, **signatures publiques inchangées** (Blueprints et tests existants en dépendent) :
   - `ServerDrainSanity` / `AuthDrainSanity` -> `PS->AuthApplySanityDelta(-Amount)`
   - `ServerRestoreSanity` -> `PS->AuthApplySanityDelta(+Amount)`
   - `ServerSetInfected` -> `PS->AuthSetInfectedPartygoer`
   - `GetSanityPercent`, `GetHealthPercent`, `IsDead`, `IsDowned`, `IsInfectedPartygoer`, `GetCarriedCredits` -> lecture PlayerState (retour sûr si PS nul).
   - `AddCarriedCredits` / `SetCarriedCredits` -> `BlueprintAuthorityOnly` + délégation.
   - `EnterDownedState` -> `PS->AuthSetStatus(Downed)` puis ragdoll/anim locale. `Revive` -> `PS->AuthRevive`.
3. Ajouter `BlueprintAuthorityOnly` à `SetHypnotized`, `SetInsideRealityAnchor`, `SetTetherPartner`.
3b. `ScavengerCharacter.h` ligne ~375 : le `float DownedTimeRemaining` répliqué viole la règle timer. Remplacer par `float DownedStartServerTime` + `float DownedDurationSeconds` répliqués sur `ALiminalPlayerState` ; `GetDownedTimeRemaining()` devient dérivé via `GetServerWorldTimeSeconds()`. Le serveur arme un `FTimerHandle` pour passer en `Dead` à expiration. Retirer ensuite la ligne correspondante de `scripts/ci/lint_baseline.txt`.
4. `ULiminalSanityPostProcessComponent` et `ALiminalHallucinationActor` : se brancher sur `PS->OnSanityChanged` dans `BeginPlay` uniquement si le pawn est `IsLocallyControlled()`. Débrancher dans `EndPlay`.
5. `ScavengerHUDWidget` : lire `ALiminalPlayerState` via `GetOwningPlayerState()`, plus le pawn.
6. `ALiminalGameMode::OnPlayerDied` : `PS->AuthSetStatus(Dead)` avant possession du `ALiminalSpectatorPawn`, puis `AuthSetStatus(Spectating)`.

## Assertions
- Build `MEG_ReclamationEditor Win64 Development` : 0 error, 0 warning.
- `grep -n "float Sanity\|float CurrentSanity" Source/MEG_Reclamation/Player/ScavengerCharacter.h` ne retourne rien.
- `python scripts/ci/lint_ue_network.py Source` : 0 erreur.
- `FMegPlayerStateAuthorityTest` et tous les tests existants (`MegReclamationTests`, `MegInteractionTests`, `MegMeleeSafetyTests`) verts.
- `Run_Automation_Tests.ps1` : 100 %.
