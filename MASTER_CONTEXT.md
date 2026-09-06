# MASTER CONTEXT — "ECHOES OF THE LIMINAL" (anciennement M.E.G. Reclamation)

Ce document est la Bible complète du projet. Il fusionne : le Game Design, l'Architecture Technique C++, le Bestiaire, le Lore, et les règles de comportement pour l'agent IA. Lis-le en entier avant de commencer toute tâche.

---

## 0. RÈGLES D'AUTONOMIE POUR L'AGENT

Tu es le Lead Gameplay Programmer sur ce projet Unreal Engine 5.8, en C++. Tu travailles en autonomie complète mais tu dois respecter ce cadre :

1. **Scope discipline** : Ne développe QUE ce qui est explicitement demandé dans le prompt de session en cours. Ce document est une bible de référence, pas une todo-list à exécuter d'un coup. Si le contexte semble suggérer d'ajouter des systèmes non demandés, NE LE FAIS PAS — signale-le en fin de réponse à la place.
2. **Compile après chaque classe créée**, via le MCP Unreal. Corrige les erreurs toi-même en lisant le log complet avant de patcher.
3. **Ne commit/ne conclus une session que si le code compile proprement**, sans nouvelle erreur ni nouveau warning majeur.
4. **Réseau Server-Authoritative strict** : aucune variable de gameplay (santé, sanity, inventaire, mort) ne doit être décidée côté client. Utilise `UFUNCTION(Server, Reliable)` pour toute action gameplay, `DOREPLIFETIME` pour la réplication d'état.
5. **Thread safety** : pour tout ce qui touche à l'audio (composant vocal), ne jamais allouer de mémoire dans le thread audio. Utiliser des buffers pré-alloués et `FCriticalSection`/`FScopeLock` si accès concurrent.
6. **Nom du module** : `MEG_Reclamation` (ou `Liminal` si renommage validé — demande confirmation avant de renommer quoi que ce soit).
7. **Ordre de développement** : respecte strictement l'ordre de la roadmap en section 8. Ne saute pas d'étape.

---

## 1. VISION & PITCH

**Genre** : Horreur d'extraction coopérative scientifique (1-4 joueurs). Unreal Engine 5.8, C++ core / Blueprint contenu.

**Pitch** : Vous êtes des "Récupérateurs" / "Techniciens de Réalité" employés par une organisation (M.E.G.) qui explore des espaces liminaux instables ("Backrooms"). Pas d'armes à feu — la philosophie est l'**Active Disempowerment** : vous survivez par la vulnérabilité, la gestion d'inventaire physique, et des outils analogiques, pas par le combat direct.

**Boucle centrale** : Hub (préparation/achat) → Insertion (ascenseur) → Scavenge (exploration/loot, gestion lumière/bruit) → Extraction (retour au point de sortie avant l'effondrement du niveau).

---

## 2. MÉCANIQUES CLÉS (ce qui différencie ce jeu)

### A. La Triade de Lumière
- **Lampe standard** : permet de voir, mais attire l'agressivité de certaines entités (Smiler).
- **Obscurité + contact visuel** : sécurité relative (fige certains monstres type "Ange Pleureur"), mais draine la Sanité.
- **Lampe UV** (item rare) : repousse activement les entités de l'ombre, consomme la batterie très vite.

### B. Mimétisme audio (Skinwalker)
Le jeu capture le micro des joueurs dans un buffer circulaire en RAM (jamais écrit sur disque). Une entité peut rejouer des fragments de voix enregistrés pour isoler un joueur en lui faisant croire qu'un allié l'appelle.

### C. Santé mentale asymétrique (client-side)
La Sanité baisse dans le noir / près d'anomalies. Sous un seuil, le client local voit des hallucinations (faux monstres, fausses portes) que les autres joueurs ne voient PAS. Force la communication et le doute.

### D. Poids et friction physique
Les objets lourds nécessitent d'être portés à deux mains (bloque l'usage d'outils), ralentissent, font du bruit. Crée une asymétrie de rôles (porteur vulnérable / garde mobile).

### E. Infection sociale
Un joueur touché par une entité "Partygoer" devient infecté : voix modifiée, objectif secret de trahison. Paranoïa sociale type Among Us.

---

## 3. ARCHITECTURE C++ — REGISTRE DE CLASSES (nomenclature officielle)

### Joueur & Gadgets (`Source/MEG_Reclamation/Player/`, `Tools/`)
- `AScavengerCharacter` (Character) : Personnage joueur récupérateur. Gère Stamina, Poids d'inventaire physique, Sanité, Santé, Lampe frontale orientable, `PhysicsHandleComponent` (saisie/dépôt d'objets lourds), Cycling et utilisation d'outils, Bruit de pas répliqué pour l'IA (`MakeNoise`), et timer d'hallucinations locales sous seuil de sanité.
- `ULiminalFootstepComponent` : Génération de sons de pas et de bruit dynamique selon les Physical Materials.
- `ULiminalSpectatorPawn` : Mode spectateur post-mortem avec déplacement libre ou caméra sur alliés survivants.
- `ABaseTool` (Actor) : Classe mère des outils analogiques avec batterie répliquée, surchauffe, cooldown et RPC serveur `ServerUseTool`.
- **Outils Opérationnels (Phase 1)** :
  - `AFlashStrobeTool` : Éblouissement et Stun conique (portée 8m, cône 45°, drain de 25% de batterie).
  - `ASonicMicrowaveTool` : Knockback physique impulsionnel (4,5m, impulsion 120 000 U, stun 2,5s).
  - `AAlmondWaterSprayTool` : Calme temporaire des entités agressives (Wretch sur 4s), restauration de sanité.
  - `AAudioDecoyTool` : Balise déployable au sol émettant du bruit périodique pour détourner les entités auditives (Hounds).
- **Outils Prévus (Phase 2)** : `ALiminalGadget` (Scanner LIDAR, Analyseur de fréquence), `ALiminalAnchor` (Ancre de réalité), Câble de liaison (Tether).
- `UVoiceMimicryComponent` (Prévu Phase 2) : Buffer audio circulaire en mémoire pour l'enregistrement et mimétisme vocal du Skinwalker.

### Objets de Gameplay & Hub (`Source/MEG_Reclamation/Objects/`)
- `ALootActor` (Actor) : Objet physique lootable (mesh, collision, physique simulée, poids, valeur en crédits, réplication physics et surbrillance diégétique au regard).
- `AExtractionZone` (Actor) : Zone d'extraction avec trigger box, compte à rebours sécurisé (5s), livraison du butin au `UQuotaManager` et transition d'état de mission.
- `ALiminalTerminalActor` (Actor) : Borne interactive du Hub. Gère le catalogue marchand `FTerminalStoreItem`, les achats (`ServerPurchaseStoreItem`), la sélection du biome (`ServerSelectBiome`), et le déclenchement de l'incursion (`ServerLaunchIncursion`).

### Données & Persistance (`Source/MEG_Reclamation/Data/`)
- `UItemData` (PrimaryDataAsset) : Définition statique d'un objet (Nom, Poids, Mesh, Valeur crédits, Son de collision).
- `FLootItem` (Struct, FTableRowBase) : Entrée DataTable (`DT_LootItems`) — `ItemId`, `DisplayName`, `WeightKg`, `CreditsValue`, `Mesh`, `CollisionSound`, `EffectTag`.
- `FQuotaState` & `FQuotaLogic` (C++ pur sans UObject) : Logique mathématique de calcul du quota, livraison et accumulation de dette (`ClampTarget`, `AddDelivered`, `TotalDue`, `IsMet`, `ApplyFailure`), 100% couverte par tests automatisés.
- `UQuotaManager` (GameInstanceSubsystem) : Subsystem server-authoritative gérant le quota actif (cible par défaut 100 crédits) et la dette globale du Hub.
- `FLiminalSaveData` & `ULiminalSaveGame` : Structure et classe SaveGame stockant `TotalBankCredits` (départ 250), `ActiveQuotaCycle` (cycle 1), `CurrentDebt`, `StoredToolIds`, et `UnlockedBiomes`.
- `ULiminalGameInstance` : Gestionnaire global de la persistance, gestion des crédits bancaires, transitions de niveaux (`TravelToMission`, `ReturnToHub`), sélection de biome (`ELevelBiome`) et sauvegarde sur disque.

### Interface Utilisateur (`Source/MEG_Reclamation/UI/`)
- `UScavengerHUDWidget` (UserWidget) : HUD analogique fournissant les getters de données pour UMG : pourcentages de Santé et Sanité, Endurance, Ratio de charge physique, Crédits portés, Nom et Batterie de l'outil actif, et indicateurs d'état critique.

### IA & Entités (`Source/MEG_Reclamation/AI/`)
- `ALiminalEntity` (Character) : Classe mère répliquée pour les entités liminales. Gère Santé, Dégâts d'attaque, Portée, Cooldown, États Stun (`ApplyStun`) et Calm (`ApplyCalm`), et attaque de mêlée.
- `EMonsterType` (Enum) : `Standard`, `Smiler`, `Hound`, `Duller`, `Clump`, `Deathmoth`, `Skinwalker`.
- `ALiminalAIController` : Contrôleur IA avec `UAIPerceptionComponent` natif (`AISense_Hearing` pour les pas/leurres, `AISense_Sight`).
- **Sous-Classes d'Entités Concrètes** :
  - `ALiminalEntity_Smiler` : Entité photophobe tapie dans l'ombre, charge si éclairée directement.
  - `ALiminalEntity_Watcher` : Silhouette immobile, inflige un drain massif de sanité si observée.
  - `ALiminalEntity_Clump` : Amas organique statique, piège et immobilise au contact.
  - `ALiminalEntity_Deathmoth` : Prédateur volant rapide, sensible aux flashs stroboscopiques.
  - `ALiminalEntity_Wretch` : Humanoïde errant guidé par le bruit, calmable au spray d'eau d'amande.

### Environnement & ProcGen (`Source/MEG_Reclamation/ProcGen/`)
- `FLiminalLayoutBuilder` & `FLiminalLayoutLibrary` : Moteur de génération procédurale déterministe (seed pure, placement AABB de salles sans collision, couloirs en L, boucles aléatoires supplémentaires, validation topologique BFS `IsEveryRoomConnected`).
- `ALiminalLevelGenerator` (Actor) : Générateur de niveau UE répliqué. Instancie la géométrie via `UInstancedStaticMeshComponent` (sols, murs, plafonds), configure les ambiances lumineuses par biome, et peuple le niveau (PlayerStarts, Extraction, Loots, Hounds, Smilers, Clumps, Watchers, Wretches, Deathmoths).
- `ELevelBiome` (Enum) : 11 biomes officiels des Backrooms.

### Modes de Jeu (`Source/MEG_Reclamation/GameModes/`)
- `ALiminalGameMode` : Mode de jeu des niveaux hostiles. Gère le cycle de vie des joueurs, le passage en spectateur, la détection de squad wipe, la validation d'extraction et les états de partie `EExtractionMatchState` (`InMission`, `ExtractionPending`, `MissionSuccess`, `SquadWiped`).
- `ALiminalLobbyGameMode` : Mode de jeu de la zone sûre (Hub). Sans monstres ni drain de sanité, point d'ancrage pour les joueurs et terminal de lancement de mission.

### Outillage Éditeur & Tests (`Source/MEG_Reclamation/Editor/`, `Tests/`)
- `MegReclamationTests.cpp` : Suite de 8 tests d'automatisation native UE sous la catégorie `Project.Functional Tests.MEG.*` (QuotaManager, QuotaDebt, ProcGenDeterminism, ProcGenConnectivity, LootItemDataTable, ProcGenRoomPlacement, SaveData, MultiBiomeLayout, AllBiomesIntegrity).
- `MegFullGameAutomator.cpp` : Commande console `MEG.BuildFullGame` créant automatiquement la DataTable `DT_LootItems` (12 objets typés) et les matériaux liminaux `/Game/Materials`.
- `HoundAIBuilder.cpp` & `LoopInputBuilder.cpp` : Constructeurs d'assets de test pour l'IA et les mappings d'Enhanced Input.

---

## 4. LE BESTIAIRE (comportements et contre-mesures)

| Entité | Statut C++ | Punit | Comportement In-Game | Contre-mesure & Outil |
|---|---|---|---|---|
| **Smiler** | Implémenté (`ALiminalEntity_Smiler`) | La lumière | Reste tapi dans l'obscurité, charge agressivement si éclairé par une lampe standard. | Éteindre la lampe frontale, utiliser le Flash Strobe (`AFlashStrobeTool`) pour l'étourdir. |
| **Hound** | Implémenté (`LiminalAIController`) | Le bruit / La course | Aveugle, réagit aux bruits de pas (`MakeNoise`) et chutes d'objets lourds. Fonce sur la source sonore. | Marcher accroupi, déployer le Leurre Audio (`AAudioDecoyTool`) pour créer une diversion. |
| **Clump** | Implémenté (`ALiminalEntity_Clump`) | L'inattention | Amas organique statique au sol, agrippe le joueur au contact et l'immobilise. | Repousser au Micro-Onde Sonique (`ASonicMicrowaveTool`) ou contourner la zone. |
| **Watcher** | Implémenté (`ALiminalEntity_Watcher`) | La curiosité | Silhouette statique dans les pièces vides ; draine agressivement la sanité si fixée du regard. | Ne pas regarder directement, fuir la pièce, utiliser le Spray d'Eau d'Amande pour régénérer la sanité. |
| **Wretch** | Implémenté (`ALiminalEntity_Wretch`) | La précipitation | Zombie errant attiré par les bruits d'impact et de course, attaque au corps-à-corps. | Calmer temporairement au Spray d'Eau d'Amande (`AAlmondWaterSprayTool`), knockback sonique. |
| **Deathmoth** | Implémenté (`ALiminalEntity_Deathmoth`) | L'immobilité | Phalène géante agressive volant vers les cibles, attaque rapide. | Éblouir au Flash Strobe pour interrompre son assaut. |
| **Skinwalker** | Implémenté (`ALiminalEntity_Skinwalker`) | La dispersion / Bruit vocal | Isole les joueurs en mimant la voix enregistrée d'un coéquipier (`UVoiceMimicryComponent`). | Mots de passe audio entre joueurs, rester groupé. |
| **Duller** | Implémenté (`ALiminalEntity_Duller`) | Absence de scanner | Invisible à l'œil nu, rôde dans les couloirs. Révélé par impulsion stroboscopique ou LIDAR. | Scanner LIDAR (`ALidarScannerTool`). |
| **Jerry** | Implémenté (`ALiminalEntity_Jerry`) | L'approche | Oiseau hypnotique, bloque les mouvements de la cible avec un rayon psionique. | Un coéquipier doit briser la transe par dégâts ou Micro-Onde Sonique. |
| **Partygoer** | Implémenté (`ALiminalEntity_Partygoer`) | Le contact social | Infection asymétrique transformant le joueur en traître (`ServerSetInfected`). | Éviter tout contact, mise en quarantaine immédiate. |

---

## 5. L'ARSENAL (outils analogiques, pas d'armes à feu)

### Outils Opérationnels (Phase 1 — Entièrement Implémentés en C++) :
- **Flash Strobe** (`AFlashStrobeTool`) : Émet un éclair aveuglant intense dans un cône de 8 mètres. Étourdit les entités pendant 3 secondes. Consomme 25% de batterie, recharge passive continue.
- **Micro-Onde Sonique** (`ASonicMicrowaveTool`) : Impulsion acoustique directionnelle à courte portée (4,5m). Applique une force d'expulsion physique de 120 000 unités et un stun de 2,5s. Surchauffe immédiate nécessitant un temps de refroidissement.
- **Spray Eau d'Amande** (`AAlmondWaterSprayTool`) : Brume apaisante sur 3,5 mètres. Calme les entités agressives (`ApplyCalm` sur 4 secondes) et restaure la sanité du joueur et des alliés à proximité.
- **Leurre Audio** (`AAudioDecoyTool`) : Radio portable analogique largable au sol. Émet des impulsions sonores répétées à fort volume pendant 15 secondes pour attirer les entités auditives (Hounds) loin de l'escouade.

### Gadgets Scientifiques & Consommables (Phase 2 — Entièrement Implémentés en C++) :
- **Scanner LIDAR** (`ALidarScannerTool`) : Balayage conique volumétrique révélant la topologie dans le noir complet et perçant le camouflage optique des entités invisibles (Dullers).
- **Analyseur de Fréquence** (`ASignalAnalyzerTool`) : Console portable diégétique avec spectrographe radio localisant le loot et les anomalies à travers les parois (rayon 25m).
- **Ancre de Réalité** (`ARealityAnchorTool`) : Dispositif lourd (30 kg) déployable au sol créant une zone safe temporaire (rayon 8,5m) où le drain de sanité et les hallucinations sont bloqués.
- **Câble de Liaison / Tether** (`ATetherTool`) : Lien physique entre deux joueurs pour empêcher la dispersion dans les zones sans visibilité et forcer la cohésion de déplacement.
- **Craie & Balises** (`AChalkMarkerTool`) : Marquage au sol à charge limitée (16 marques) pour prévenir la désorientation dans les dédales non-euclidiens.

---

## 6. LES 11 BIOMES OFFICIELS (implémentés dans `ELevelBiome`)

Le générateur procédural `ALiminalLevelGenerator` configure dynamiquement l'éclairage, la composition des salles et le peuplement monstre selon l'enum `ELevelBiome` :

| ID Enum | Nom du Biome | Ambiance Lumineuse | Intensité | Entités Spécifiques Présentes |
|---|---|---|---|---|
| `Level0_YellowLobby` | Niveau 0 — Le Lobby Jaune | Jaune chaud (1.0, 0.95, 0.75) | 2000 lux | Hound, Smiler |
| `Level1_HabitableZone` | Niveau 1 — Zone Habitable | Béton froid bleuté (0.85, 0.92, 1.0) | 1600 lux | Hound, Clump |
| `Level2_PipeDreams` | Niveau 2 — Tuyauteries & Vapeur | Ambre industriel (1.0, 0.45, 0.12) | 1400 lux | Hound, Clump, Wretch |
| `Level3_ElectricalStation` | Niveau 3 — Station Électrique | Cyan électrique (0.25, 0.75, 1.0) | 1750 lux | Hound, Smiler |
| `Level4_AbandonedOffice` | Niveau 4 — Bureaux Abandonnés | Blanc chirurgical (0.88, 0.96, 0.94) | 2200 lux | Hound, Watcher, Smiler |
| `Level6_LightsOut` | Niveau 6 — Noir Absolu | Obscurité quasi-totale (0.01, 0.01, 0.02) | 20 lux | Hound, Wretch, Smiler |
| `Level8_CaveSystem` | Niveau 8 — Réseau de Cavernes | Minéral sombre (0.45, 0.40, 0.35) | 700 lux | Hound, Deathmoth, Clump |
| `Level9_DarkSuburbs` | Niveau 9 — Le Faubourg Obscur | Nuit bleutée brumeuse (0.20, 0.25, 0.55) | 1000 lux | Hound, Wretch |
| `Level10_WheatFields` | Niveau 10 — Les Champs de Blé | Ciel crépusculaire doré (1.0, 0.70, 0.35) | 1900 lux | Hound, Deathmoth |
| `Level37_Poolrooms` | Niveau 37 — Les Poolrooms | Turquoise aquatique (0.65, 0.95, 1.0) | 1800 lux | Hound, Smiler |
| `LevelRun_RunForYourLife` | Niveau ! — Fuyez pour survivre | Alarme rouge sang (1.0, 0.05, 0.05) | 3500 lux | Hound, Smiler, Wretch (sprint pur) |

---

## 7. PROGRESSION, ÉCONOMIE & HUB

- **Économie Initiale** : La sauvegarde `FLiminalSaveData` démarre avec 250 crédits en banque, au cycle de quota 1.
- **Cycle du Quota** : La cible de base est fixée à 100 crédits. Chaque mission réussie ajoute la valeur des objets rapportés (`UQuotaManager::AddDeliveredValue`). En cas d'échec de cycle, le déficit est automatiquement reporté en dette additionnelle (`FQuotaLogic::ApplyFailure`).
- **Terminal Marchand** : Les crédits servent à acheter des outils d'escouade (coût standard 100 crédits) via `ALiminalTerminalActor::ServerPurchaseStoreItem`.
- **Déverrouillage des Biomes** : La persistance conserve les biomes débloqués (`UnlockedBiomes`), sélectionnables au Hub pour la prochaine expédition.
- **Évolution Visuelle du Hub** : Composant `ULiminalHubProgressionComponent` faisant évoluer la Base Alpha à travers 3 paliers structurels (Campement de fortune → Avant-poste fortifié → Laboratoire de confinement scientifique M.E.G.).

---

## 8. ROADMAP DE DÉVELOPPEMENT — STATUT DE RÉALISATION

### Jalons Entièrement Complétés [x] :
- [x] **Étape 1 : Fondations Données & Économie** (`UItemData`, `FLootItem`, `FQuotaLogic`, `UQuotaManager`, `FLiminalSaveData`, `ULiminalGameInstance`).
- [x] **Étape 2 : Arsenal Opérationnel** (`ABaseTool` + `AFlashStrobeTool`, `ASonicMicrowaveTool`, `AAlmondWaterSprayTool`, `AAudioDecoyTool`).
- [x] **Étape 3 : Personnage Récupérateur** (`AScavengerCharacter`, endurance, friction du poids, sanité, pas, lampe, saisie physique, cycling d'outils, gestion de l'hypnose et tether).
- [x] **Étape 4 : Système d'IA et Bestiaire Complet (10 Entités)** (`ALiminalEntity`, `ALiminalAIController`, Smiler, Hound, Clump, Watcher, Deathmoth, Wretch, Skinwalker, Partygoer, Duller, Jerry).
- [x] **Étape 5 : Physique Chaos & Manipulation** (`PhysicsHandleComponent`, `ALootActor` physique avec réplication et highlight).
- [x] **Étape 6 : Boucle d'Extraction Jouable** (`ALiminalGameMode`, `ALiminalLobbyGameMode`, `AExtractionZone`, `ALiminalTerminalActor`, maps `Lvl_Loop` et `Lvl_ProcGen`).
- [x] **Étape 7 : Validation multijoueur approfondie & Tests d'automatisation** (`MegReclamationTests`, validation de synchronisation, simulation multi-clients, Listen Server autoritaire, tous tests réussis avec code 0).
- [x] **Étape 8 : Génération Procédurale** (`FLiminalLayoutBuilder`, `ALiminalLevelGenerator`, 11 biomes avec éclairages dynamiques et instanciation de tuiles).
- [x] **Étape 9 : Audio spatialisé & Metasounds** (`ULiminalAudioSubsystem`, profils acoustiques pour les 11 biomes, réverbération dynamique, stingers procéduraux, capture micro phonique vers perception IA).
- [x] **Étape 10 : Sanité avancée 2.0 & Hallucinations** (`LiminalSanityTypes`, `ULiminalSanityPostProcessComponent`, `ALiminalHallucinationActor`, alertes corrompues diégétiques dans `UScavengerHUDWidget`).
- [x] **Étape 11 : Systèmes avancés de Phase 2** (`UVoiceMimicryComponent` buffer circulaire audio RAM thread-safe, bestiaire avancé Skinwalker/Partygoer/Duller/Jerry, 5 outils Phase 2 LIDAR/Signal/Ancre/Tether/Craie, `ULiminalHubProgressionComponent` évolution visuelle Base Alpha).
- [x] **Interface & HUD** (`UScavengerHUDWidget` avec getters analogiques, alertes corrompues et indicateurs d'hallucinations).
- [x] **Automatisation & Qualité** (Suite complète de tests unitaires/fonctionnels `MegReclamationTests` validée à 100%, commande éditeur `MEG.BuildFullGame`).

<!-- GOAL_COMPLETE -->

---

## 9. TON NARRATIF (pour tout texte in-game généré)

- Rapports de mission : froid, clinique, bureaucratique.
- Notes de survivants : fragmenté, désespéré, répétitif.
- Descriptions de lieux : nostalgique et dérangeant (Anemoia).
- **Éviter absolument** : jargon gaming dans les textes narratifs (pas de "HP", "Aggro", "Spawn" dans le lore), humour type Marvel, sur-explication du lore (garder le mystère).
