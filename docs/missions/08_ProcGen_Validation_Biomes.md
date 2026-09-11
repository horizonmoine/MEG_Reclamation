# MISSION 08 — Génération procédurale déterministe, validée, 11 biomes

Respecte `.agents/rules/ue5_cpp_network_rules.md`, `AGENTS.md` section `procgen_designer` et `docs/design/LEVEL_DESIGN_BIBLE.md` sections 4 à 6.

## Contexte à lire (obligatoire avant tout code)
`ProcGen/LiminalLevelGenerator.h/.cpp`, `ProcGen/LiminalModularDungeonGenerator.h/.cpp`, `ProcGen/LiminalLayoutLibrary.h/.cpp`, `ProcGen/LiminalTileSet.h`, `ProcGen/LiminalTileData.h`, `ProcGen/LiminalTileTypes.h`, `ProcGen/LiminalRoomTemplate.h`, `ProcGen/LiminalDecoratorSubsystem.h`, `ProcGen/LiminalStreamingManager.h`, `Tests/MegProcGenSafetyTests.cpp`, `Data/LiminalGameInstance.h` (`ELevelBiome`).

## Reste à faire
1. **Seed unique** : `ALiminalLevelGenerator::SetBiome` et `Generate` consomment `ALiminalGameState::GetMissionSeed()` via `FRandomStream`. Supprimer tout `FMath::Rand`/`FMath::FRand` du dossier `ProcGen/` (grep).
2. **Regénération client** : le générateur écoute `OnRep_MissionSeed` (ou lit la seed dans `BeginPlay` côté client) et produit la même géométrie ISM localement. Aucun mesh de couloir répliqué. Les actors gameplay (portes, loot, disjoncteurs) restent spawnés par le serveur.
3. **Validation** dans `ULiminalLayoutLibrary` (fonctions pures, testables) : `ValidateLayout(const FLayout&, FLayoutReport&)` vérifiant : distance géodésique spawn->extraction ≥ 8000 cm et ≥ 3 virages ; connexité A* de 100 % des cellules loot ; culs-de-sac ≤ 15 % et profondeur ≤ 2 ; ≥ 2 boucles indépendantes ; portes verrouillées hors chemin critique. En cas d'échec, régénérer avec `Seed + 1` (max 8 tentatives, logué).
4. **Grille par N** : 12/14/17/20 cellules selon `GameState->PlayerArray.Num()` au moment de `StartIncursion`.
5. **Salles spéciales** garanties par run (section 4.6 de la bible) : 1 `ALiminalBreakerActor`, 1 puzzle (`ALiminalValvePuzzleActor` ou `ALiminalFuseBoxActor`), 2-4 `ALiminalHidingSpot`, 1 `ALiminalVentActor` reliant deux boucles, 0-1 `ALiminalSafeZoneVolume` (`SanityRestorePerSecond = 1`).
6. **Loot** : valeur totale générée = 1.6 x `Q(k,N)` (via `FQuotaLogic`), répartition 60/30/10 léger/lourd/rare, tirée dans `DT_LootItems` par tier de biome.
7. **Biomes** : une `ULiminalTileSet` par biome (11) avec palette, tuiles dominantes et roster d'après la table de la bible. `ULiminalDecoratorSubsystem` applique l'ambiance (néons, flicker, brume, eau) déterministe par seed. Les assets manquants sont remplacés par des placeholders nommés `SM_PH_<Biome>_<Tuile>` et listés dans `docs/design/ASSET_BACKLOG.md` pour `blender_3d_modeller`.

## Assertions
- Build 0 error / 0 warning. Lint CI 0 erreur.
- `Tests/MegProcGenSafetyTests.cpp` étendu : 100 seeds x 4 valeurs de N : 0 échec de `ValidateLayout` après régénération ; même seed => layout identique (hash des cellules).
- Test : deux générateurs (simulant serveur et client) avec la même seed produisent le même hash.
- `grep -rn "FMath::Rand\|FMath::FRand\|FMath::RandRange" Source/MEG_Reclamation/ProcGen/` vide.
