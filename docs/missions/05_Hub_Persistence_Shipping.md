# MISSION 05 — Boucle complète Hub -> Incursion -> Debrief, persistance, certification Shipping

Respecte `.agents/rules/ue5_cpp_network_rules.md` et `AGENTS.md` section `qa_release_sentinel`. Prérequis : Missions 01 à 04.

## Contexte
Lis `Hub/LiminalHubProgressionComponent.h`, `Data/QuotaManager.h/.cpp`, `Data/LiminalGameInstance.h/.cpp`, `Network/LiminalSessionManager.h`, `UI/LiminalDebriefHUD.h`, `GameModes/LiminalLobbyGameMode.h`, `Editor/MegFullGameAutomator.cpp`, `Run_Auto_Check.ps1`, `Package_Shipping_Build.ps1`.

## Outputs attendus
1. `ULiminalGameInstance` : `USTRUCT FLiminalRunSummary` (crédits extraits par joueur via `FUniqueNetIdRepl`, quota atteint selon la formule Q(k,N) du GEMINI.md, seed, durée, morts, Dog Tags rapatriés +50 BR, cadavres abandonnés -80 BR) rempli par `ALiminalGameMode::FinishMission`, puis Client RPC `ClientReceiveRunSummary` sur chaque PlayerController pour le debrief local.
2. `bUseSeamlessTravel = true` entre Hub et Biome ; `ALiminalPlayerState` conserve `CarriedCredits` via `CopyProperties`. `UQuotaManager` crédite la banque équipe sur serveur, résultat dans `ALiminalGameState::TeamBankCredits`.
3. `ALiminalLobbyGameMode` : readiness via `bIsReady` du PlayerState (Server RPC `ServerSetReady` sur le PlayerController) ; l'hôte lance `StartIncursion` uniquement quand tous prêts.
4. `ULiminalProfileSave : public USaveGame` (crédits banque, améliorations Hub, cycle k) écrit par l'hôte au retour au Hub, chargé dans `Init` du GameInstance. Aucune écriture disque côté client non-hôte.
5. Shipping : tout debug (`AddOnScreenDebugMessage`, `DrawDebug*`, commandes console custom) sous `#if !UE_BUILD_SHIPPING`. `Builds/Windows/MEG_Reclamation.exe` démarre sans console.
6. `Run_Auto_Check.ps1` : 3 nouveaux contrôles : présence de `ALiminalPlayerState`, absence de float répliqué `*TimeRemaining`, aucun Server RPC dans `GameModes/`.

## Assertions
- Build Development Editor, Development Win64 ET Shipping Win64 : 0 error, 0 warning chacun.
- `Package_Shipping_Build.ps1` sans erreur de cook, `.exe` lancé 60 s sans crash (log propre).
- `Tests/MegRunSummaryTests.cpp` : `FinishMission(true)` avec 2 PlayerStates produit 2 entrées et `TeamBankCredits` = somme des `CarriedCredits`.
- `Run_Automation_Tests.ps1` : 100 %. `Run_Auto_Check.ps1` : tous verts (30 + 3 minimum). Lint CI : 0 erreur.
