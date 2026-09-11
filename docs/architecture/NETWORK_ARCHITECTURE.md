# Architecture réseau Server-Authoritative — M.E.G. : Reclamation

Boucle : Hub Base Alpha -> Sas d'incursion -> Biomes procéduraux -> Loot -> Sanité/Survie -> Extraction ou Reality Collapse (8 min).

## Principe unique
**Le serveur décide, le client demande et affiche.** Toute mutation d'état = `HasAuthority()` ou Server RPC validé. Tout ce que le client voit = propriété répliquée + `OnRep_`.

## État des lieux (audit du code existant)
| Constat | Risque | Correction | État |
|---|---|---|---|
| Aucune `APlayerState` custom. Sanité, HP, downed, infection, crédits vivent sur `AScavengerCharacter` (640 lignes) | Perte totale à la destruction du pawn (mort, spectateur, travel). Debrief sans source de vérité | Mission 01 : `ALiminalPlayerState` | Classe livrée + test. Migration du Character à faire |
| `ALiminalGameMode::RealityCollapseTimer` décrémenté au Tick, invisible des clients ; `CurrentStability` float répliqué | Dérive de latence, HUD saccadé, fin de partie non déterministe | Mission 02 : timestamp serveur dans le GameState | `AuthStartCollapseTimer` + `GetCollapseTimeRemaining` livrés, démarrés par le GameMode. Suppression du Tick à faire |
| Seed procgen tirée localement (`FMath::Rand()` dans le GameMode) | Late-join impossible à resynchroniser | `ALiminalGameState::MissionSeed` | Livré |
| `SetHypnotized`, `SetInsideRealityAnchor`, `SetTetherPartner` sont `BlueprintCallable` sans garde d'autorité | Divergence client/serveur, triche triviale | Mission 01 : `BlueprintAuthorityOnly` | À faire |
| Inventaire Tetris en `TArray` classique | Renvoi complet du tableau à chaque changement | Mission 03 : `FFastArraySerializer` | Types `FLiminalInventoryList` livrés + test. Branchement composant à faire |
| Stimuli outils (flash, decoy) émis côté client | L'IA serveur n'entend jamais le decoy | Mission 04 : `ULiminalStimulusSubsystem` serveur | Subsystem livré + test. Branchement AIControllers/outils à faire |

## Fichiers livrés par la MR !1 (fondations)
- `Player/LiminalPlayerState.h/.cpp`
- `GameModes/LiminalGameState.h/.cpp` (étendu), `GameModes/LiminalGameMode.cpp` (PlayerStateClass, seed, timer)
- `AI/LiminalStimulusSubsystem.h/.cpp`
- `Inventory/LiminalInventoryTypes.h/.cpp`
- `Tests/MegNetworkFoundationTests.cpp` (4 tests : `MEG.Network.*`)
- `MEG_Reclamation.Build.cs` (+ `NetCore`)

## Matrice des responsabilités
| Classe | Existe sur | Possède (source de vérité) | Ne fait JAMAIS |
|---|---|---|---|
| `ALiminalGameMode` | Serveur | Règles, spawn/possession, transitions de phase, seed procgen, crédit du loot extrait au quota | Propriété répliquée, UI, référence côté client |
| `ALiminalGameState` | Tous | `MissionPhase`, `CollapseStartServerTime` + `CollapseDurationSeconds`, `MissionSeed`, stabilité, blackout, `TeamBankCredits` | Décider (délègue au GameMode), tick de décrémentation |
| `ALiminalPlayerState` (à créer) | Tous | Sanité, HP, `EScavengerStatus`, crédits portés, infection, stats debrief, `bIsReady` | Mouvement, input |
| `AScavengerCharacter` | Serveur + owner autonome | Mouvement, stamina prédite, poids, mains (carry), point d'entrée des Server RPC d'interaction | Stocker un état de session, appliquer des dégâts localement |
| `AMEG_ReclamationPlayerController` | Serveur + owner | Input, HUD, caméra, spectateur, Client RPC (`ClientReceiveRunSummary`, `ClientShowFakeAlert`) | Répliquer du gameplay aux autres |
| `ULiminalTetrisInventoryComponent` | Owner-only | Grille via `FFastArraySerializer`, validation de placement serveur | Être lu par les autres clients (seul poids/valeur remonte au PlayerState) |
| Subsystems (`Event`, `Audio`, `Decorator`, `Stimulus`) | Locaux | Effets, audio, décoration déterministe par seed ; `Stimulus` serveur uniquement | État gameplay répliqué |
| `ALiminalAirlockActor`, `AExtractionZone`, `ALootActor`, `ABaseTool`, `ALiminalEntity` | Répliqués | Leur propre état via `OnRep_` | Accepter une interaction sans Server RPC du Character + check distance |

## Flux de référence : ramasser un artefact
1. Input client -> `AScavengerCharacter::ServerRequestPickup(ALootActor*)` (Reliable, WithValidation).
2. Serveur : distance < 250 cm, ligne de vue, loot non détenu, poids OK.
3. `Inventory->TryPlace()` ou attache mains (loot lourd) -> `LootActor->HolderCharacter` répliqué -> `PlayerState->CarriedValue`.
4. `OnRep_` côté clients pour feedback. Aucun état modifié côté client avant retour serveur (animation cosmétique locale tolérée).

## Machine à états de mission
`Hub -> Airlock -> Incursion -> Collapsing -> {Extracted | Failed}`
- Transitions uniquement via `ALiminalGameMode::SetMissionPhase` (serveur). Transitions invalides loguées et refusées.
- `StartIncursion()` : écrit `CollapseStartServerTime = GetServerWorldTimeSeconds()` et arme `FTimerHandle CollapseTimerHandle` (480 s) -> `FinishMission(false)`.
- Client : `Remaining = Duration - (ServerNow - Start)`. Aucun client ne déclenche une fin de partie.
- Late-join : lit le GameState, aucun RPC dédié.

## Dix pièges UE5 à anticiper
1. Timer répliqué qui tique : toujours un timestamp, fin de partie par `FTimerHandle` serveur.
2. Inventaire `TArray` répliqué : `FFastArraySerializer`, `COND_OwnerOnly`, item porté = actor attaché, pas une entrée.
3. RPC Server appelé depuis un actor non possédé : ignoré silencieusement. Uniquement pawn possédé / PlayerController / PlayerState.
4. Sanité asymétrique : valeur serveur, hallucinations spawnées localement sur `OnRep_Sanity`, `bReplicates = false`.
5. Downed/Revive : état dans le PlayerState, pawn conservé en ragdoll répliqué, validation `CanReviveTarget` serveur.
6. Seed procédural : générée en `InitGame`, écrite dans le GameState avant `BeginPlay` client, géométrie ISM jamais répliquée.
7. Dormancy : `DORM_DormantAll` sur loot au sol, `FlushNetDormancy` au pickup.
8. Pointeurs dangling : `TWeakObjectPtr` pour toute référence non possédée.
9. Outils sans armes : effet monde = Server RPC -> stimulus IA serveur -> Multicast cosmétique.
10. Hub persistant : `bUseSeamlessTravel = true`, `CopyProperties`/`OverrideWith` sur le PlayerState, `USaveGame` écrit par l'hôte seul.
