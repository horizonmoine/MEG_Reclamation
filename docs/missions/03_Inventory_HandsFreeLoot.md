# MISSION 03 — Brancher FLiminalInventoryList dans le composant Tetris et le loot mains-libres

Respecte `.agents/rules/ue5_cpp_network_rules.md`. Prérequis : Missions 01 et 02.

## Déjà livré (ne pas recréer)
- `Inventory/LiminalInventoryTypes.h/.cpp` : `FLiminalInventoryEntry` (`FFastArraySerializerItem` : `EntryId`, `ItemId`, `GridPos`, `Size`, `bRotated`, `WeightKg`, `Value`) et `FLiminalInventoryList` (`FFastArraySerializer`, `NetDeltaSerialize`, `TStructOpsTypeTraits`).
- Logique serveur prête : `CanPlace`, `AddEntry`, `RemoveEntry`, `MoveEntry`, `FindEntry`, `FindFreeSlot`, `GetTotalWeightKg`, `GetTotalValue`. Callback client `OnReplicatedChange`.
- `NetCore` ajouté au `Build.cs`.
- Test : `FMegInventoryListTest`.

## Contexte à lire
`Inventory/LiminalTetrisInventory.h/.cpp`, `Objects/LootActor.h/.cpp`, `Data/ItemData.h`, et dans `ScavengerCharacter` : `InputGrab`, `InputRelease`, `InputThrowLoot`, `DropCarriedLootOnGround`, `ServerAddInventoryWeight`, `ServerRemoveInventoryWeight`, `GetWeightRatio`.
Le jeu n'a AUCUNE arme : les mains portent soit un outil (`ABaseTool`) soit un loot lourd, jamais les deux.

## Reste à faire
1. `ULiminalTetrisInventoryComponent` : remplacer le stockage actuel par `UPROPERTY(Replicated) FLiminalInventoryList Inventory;` avec `DOREPLIFETIME_CONDITION(..., COND_OwnerOnly)`. `SetIsReplicatedByDefault(true)` dans le constructeur. Dans `BeginPlay`, `Inventory.OnReplicatedChange = [this]() { OnInventoryChanged.Broadcast(); }`. Conserver l'API publique existante en façade.
2. Mutations serveur du composant : `bool AuthTryAddItem(FName ItemId, FIntPoint Size, float WeightKg, int32 Value, int32& OutEntryId)` (utilise `FindFreeSlot` puis `AddEntry`), `AuthRemoveItem(int32 EntryId)`, `AuthMoveItem(EntryId, Pos, bRotated)`. Chaque mutation met à jour le poids du Character et `PS->AuthSetCarriedCredits(Inventory.GetTotalValue())`.
3. `AScavengerCharacter` : Server RPC `ServerRequestPickup(ALootActor*)`, `ServerRequestStow()`, `ServerRequestDrop()`, `ServerRequestThrow(FVector Direction)`, `ServerRequestMoveItem(int32 EntryId, FIntPoint Pos, bool bRotated)`. Validation : distance < 250 cm, loot non détenu, poids <= capacité. `InputGrab/Release/Throw` deviennent des appels aux RPC.
4. `ALootActor` : `bReplicates`, `SetReplicateMovement(true)`, `TWeakObjectPtr<AScavengerCharacter> HolderCharacter` répliqué + `OnRep_Holder` (attache socket `hand_r`, physique off côté client). `NetDormancy = DORM_DormantAll`, `FlushNetDormancy()` à chaque mutation.

## Assertions
- 0 error, 0 warning. Lint CI : 0 erreur (aucun `TArray<UObject*>`/`TMap` répliqué).
- `FMegInventoryListTest` et `MegInteractionTests` verts.
- Nouveau test `Tests/MegInventoryComponentTests.cpp` : `AuthTryAddItem` x3 puis retrait du 2e conserve les ids ; poids Character cohérent ; `CarriedCredits` du PlayerState = `GetTotalValue()`.
