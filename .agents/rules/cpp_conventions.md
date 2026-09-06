# Règles C++ Unreal Engine 5.8 pour MEG_Reclamation

1. Préfixes de types :
   - `A` pour les Actors (`AScavengerCharacter`, `ALiminalAirlockActor`).
   - `U` pour les UObjects / Components (`ULiminalInventoryComponent`, `ULiminalProximityVoiceComponent`).
   - `F` pour les structs / classes pures (`FLiminalRoomNode`, `FLiminalItemData`).
   - `E` pour les enums (`EPlayerSanityState`, `EBiomeType`).

2. Sécurité mémoire et GC :
   - Toujours initialiser les membres UPROPERTY avec nullptr ou valeurs par défaut.
   - Utiliser `TObjectPtr<T>` pour les pointeurs gérés par l'UObject system.
   - Vérifier `IsValid(Pointeur)` ou `Pointeur != nullptr` avant tout appel de méthode.

3. Réseau & Réplication :
   - Les actions modifiant l'état du jeu (prise d'objet, dégâts, utilisation d'outil) doivent être validées côté Serveur (`ServerRPC` avec `WithValidation`).
   - Utiliser `DOREPLIFETIME` et `DOREPLIFETIME_CONDITION` pour limiter la bande passante.

4. Performance :
   - Préférer les itérateurs const et références constantes (`const FVector&`).
   - Ne jamais allouer dynamiquement dans les boucles `Tick()`.
