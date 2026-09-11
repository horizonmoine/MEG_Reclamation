# Règles d'ingénierie C++ UE 5.8 — M.E.G. : Reclamation

> Fichier de référence unique pour Gemini CLI, OpenCode, Antigravity, Cursor et Claude.
> Cible : MSVC 14.44, Win64, Development Editor / Development / Shipping. **Zéro warning.**
> Tu ne réponds jamais par des pseudo-codes ou des `...` : chaque fichier livré est complet et compile.

## 0. Avant d'écrire
- Lis TOUJOURS les headers existants concernés dans `Source/MEG_Reclamation/**` avant de créer une classe.
- Ne recrée jamais une classe existante : `ALiminalGameMode`, `ALiminalGameState` (phases, timer, seed, banque),
  `ALiminalPlayerState` (sanité, HP, statut, crédits), `AScavengerCharacter`, `ULiminalTetrisInventoryComponent`,
  `FLiminalInventoryList` / `FLiminalInventoryEntry` (FastArray), `ULiminalStimulusSubsystem` (bruit/lumière serveur),
  `ALiminalAirlockActor`, `AExtractionZone`, `ALootActor`, `ABaseTool`, `ALiminalEntity`, `ULiminalEventSubsystem`,
  `ULiminalAudioSubsystem`, `UQuotaManager`. Étends ou modifie.
- Toute mutation d'état de session joueur passe par les méthodes `Auth*` de `ALiminalPlayerState`.
  Toute transition de phase passe par `ALiminalGameState::AuthSetMissionPhase`, appelée uniquement par le GameMode.
- Si une API UE te semble incertaine, cherche la signature dans `F:\UE_5.8\Engine\Source` avant de l'utiliser.
  N'invente JAMAIS une fonction, un macro ou un include.
- Un type UCLASS/USTRUCT par fichier, dans le sous-dossier de domaine existant
  (`Player/`, `Inventory/`, `Objects/`, `GameModes/`, `Sanity/`, `ProcGen/`, `Tools/`, `AI/`, `UI/`, `Tests/`).

## 1. Nommage (Epic Coding Standard, non négociable)
- Préfixes : `A` Actor, `U` UObject/Component/Subsystem, `F` struct, `E` enum class, `I` interface, `S` Slate, `T` template.
- Booléens : préfixe `b` (`bIsDowned`). Enums : `enum class EMissionPhase : uint8` + `UENUM(BlueprintType)`.
- Fonctions PascalCase, verbe en premier (`RequestPickup`, `TryPlaceItem`).
- Server RPC : préfixe `Server`. Client RPC : `Client`. Multicast : `Multicast`. OnRep : `OnRep_<Nom>` avec `UFUNCTION()`.
- Fichiers : `<NomSansPréfixe>.h/.cpp`. Macro export : `MEG_RECLAMATION_API` sur toute classe/struct exposée.

## 2. Squelette de header obligatoire
```cpp
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"      // UN seul include parent
#include "MonActor.generated.h"      // TOUJOURS dernier include

class UFoo;                          // forward declarations pour tout le reste
struct FBar;

UCLASS()
class MEG_RECLAMATION_API AMonActor : public AActor
{
	GENERATED_BODY()
public:
	AMonActor();
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
```
- Aucun include lourd dans les `.h` (`Engine/Engine.h`, `Kismet/*.h`, `Components/*.h` interdits) : forward declare, include dans le `.cpp`.
- Ordre `.cpp` : header propre, moteur, projet. `#include "Net/UnrealNetwork.h"` dès qu'une propriété est répliquée.
- Indentation tabulations, accolades Allman (style Epic).

## 3. Mémoire & pointeurs
- Membres `UPROPERTY` vers UObject : `TObjectPtr<T>`. Jamais de pointeur brut membre.
- Références sans ownership (cible, partenaire tether, loot regardé) : `TWeakObjectPtr<T>`, tester `.IsValid()`.
- Classes de spawn : `TSubclassOf<T>`. Assets lourds : `TSoftObjectPtr<T>` / `TSoftClassPtr<T>`.
- Aucun `new`/`delete`, aucun `std::` container ou smart pointer. `TArray`, `TMap`, `TSet`, `FString`, `FName`, `FText` uniquement.
- Composants : `CreateDefaultSubobject<T>(TEXT("Nom"))` dans le constructeur uniquement. Pas de `GetWorld()` dans un constructeur.
- Spawn : `GetWorld()->SpawnActor<T>()` ou `SpawnActorDeferred` + `FinishSpawning`.
- Garde systématique : `if (!IsValid(Obj)) { return; }`. `check()` interdit en gameplay, `ensureMsgf` autorisé.
- `FTimerHandle` toujours membre de classe, jamais variable locale.

## 4. Réplication : modèle 100 % Server-Authoritative
- **Le serveur décide, le client demande et affiche.** Toute mutation d'état gameplay est dans `if (!HasAuthority()) { return; }`
  ou est un `UFUNCTION(Server, Reliable, WithValidation)`.
- Chaque Server RPC a `_Implementation` ET `_Validate` (`_Validate` retourne `false` uniquement en cas de triche évidente, jamais pour une règle de gameplay).
- Server RPC déclarés SEULEMENT sur des classes possédées : `APawn`, `APlayerController`, `APlayerState` ou un composant dont l'owner est l'un des trois.
  **Jamais sur `AGameStateBase`, `AGameModeBase` ou un actor monde.**
- Propriétés répliquées : `UPROPERTY(ReplicatedUsing = OnRep_X)` ; enregistrement dans `GetLifetimeReplicatedProps` via `DOREPLIFETIME` ou
  `DOREPLIFETIME_CONDITION(..., COND_OwnerOnly)`. Appeler `Super::GetLifetimeReplicatedProps`.
- Sur listen server, `OnRep_` n'est PAS appelé automatiquement : l'appeler manuellement après mutation si un effet local est requis.
- **Timers synchronisés** : répliquer un timestamp `float StartServerTime` obtenu via `GetWorld()->GetGameState()->GetServerWorldTimeSeconds()`.
  Interdit : un `float` répliqué nommé `*TimeRemaining` ou décrémenté au tick.
- Tableaux d'items : `FFastArraySerializer` + `FFastArraySerializerItem`. Jamais `TArray<UObject*>` ni `TMap` répliqués.
- `bReplicates = true` dans le constructeur de tout actor réseau ; `SetReplicateMovement(true)` uniquement si physique.
- Actors statiques nombreux (loot au sol) : `NetDormancy = DORM_DormantAll`, `FlushNetDormancy()` à toute mutation.
- Aucune logique gameplay dans `Tick` côté client. Cosmétique locale uniquement.
- Tout `BlueprintCallable` public qui mute un état est `BlueprintAuthorityOnly` OU redirige vers un Server RPC.
- Effets monde des outils (flash, bruit) : Server RPC owner -> validation -> stimulus IA serveur -> `Multicast` cosmétique.

## 5. Conventions gameplay projet
- Le joueur n'a aucune arme. Aucun système de dégâts sortants joueur -> entité.
- Sanité, HP, statut, crédits portés, infection : source de vérité `ALiminalPlayerState`. Le pawn lit et affiche.
- Hallucinations et post-process de sanité : spawn LOCAL owner-only, `bReplicates = false`.
- Dégâts d'entités appliqués uniquement dans les `AnimNotify` d'attaque, côté serveur.
- Unités : centimètres, secondes, kg. Constantes de tuning en `UPROPERTY(EditDefaultsOnly, Category="...")`, aucun magic number dans le `.cpp`.
- Logs : `UE_LOG(LogMEG, Verbose|Log|Warning, TEXT("[%s] ..."), *GetName())`.
- `FText` pour l'UI, `FName` pour les IDs, `FString` pour le debug.

## 6. Interdictions absolues
- `GetAllActorsOfClass` dans Tick ou code réseau chaud.
- Casts en chaîne : un `Cast<>`, un `IsValid`, sinon return.
- `GEngine->AddOnScreenDebugMessage`, `DrawDebug*` hors `#if !UE_BUILD_SHIPPING`.
- `delete this`, `static` mutables, globales.
- Nouvelle dépendance dans `MEG_Reclamation.Build.cs` sans commentaire justificatif.
- Modification de `Config/*.ini` ou `*.uproject` sans validation humaine explicite.

## 7. Format de livraison d'une mission
1. Liste des fichiers créés/modifiés avec chemin complet.
2. Code complet de chaque fichier.
3. Section **Compilation** avec la commande exacte :
   `F:\UE_5.8\Engine\Build\BatchFiles\Build.bat MEG_ReclamationEditor Win64 Development -Project="F:\MEG_Reclamation\MEG_Reclamation.uproject" -WaitMutex`
   et la sortie attendue `0 error(s), 0 warning(s)`.
4. Un test `IMPLEMENT_SIMPLE_AUTOMATION_TEST` dans `Source/MEG_Reclamation/Tests/` couvrant le cas nominal et un cas de refus d'autorité.
5. Exécution de `python scripts/ci/lint_ue_network.py Source` : 0 erreur.
6. Exécution de `Run_Automation_Tests.ps1` (100 %) puis `Run_Auto_Check.ps1` (tous les contrôles verts).
7. Ne touche à aucun fichier hors périmètre de la mission.
