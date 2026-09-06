# ===================================================================
# PARTIE 1 — LE JEU (GDD CANONIQUE)
# ===================================================================

## 1.1 Pitch en une phrase
Un jeu d'horreur d'extraction coopératif (1-4 joueurs) où des "Récupérateurs" du M.E.G. (Major Explorer Group) s'enfoncent dans les Backrooms sans armes à feu, équipés uniquement d'outils de maintenance, pour ramener des ressources et faire évoluer leur base — jusqu'à un jour, peut-être, s'échapper pour de bon.

## 1.2 Piliers de design (non négociables)
| Pilier | Ce que ça veut dire concrètement |
|---|---|
| **Active Disempowerment** | Le joueur est vulnérable par design. Pas de barre de vie type FPS, pas de combat frontal viable. |
| **Zéro arme à feu** | Résolu définitivement. Tout gadget qui n'est pas une arme létale à distance reste autorisé — Lidar, Ancre de Réalité, Résonateur no-clip inclus. |
| **Coop obligatoire par la friction physique** | Poids des objets, portage à deux mains, un joueur "mule" + un joueur "escorte". |
| **La peur vient des choix, pas des scripts** | Lumière ou noir ? Porter le loot ou courir plus vite ? Vraiment ton ami qui t'appelle ? |

## 1.3 Boucle de jeu (Core Loop)
```text
HUB (sécurisé) → achat/prépa
   ↓
INSERTION (ascenseur, Seamless Travel serveur)
   ↓
SCAVENGE (loot, identification des menaces)
   ↓
SURVIE (gestion lumière / bruit / sanité / poids)
   ↓
EXTRACTION (timer de stabilité, ascenseur de sortie)
   ↓
retour HUB → dépense du loot → évolution du Hub → repeat
```

## 1.4 Le joueur (ALiminalSurvivor)
Stats répliquées serveur-autoritaire :
- **Sanity (0-100)** : baisse dans le noir, près des entités, avec le bruit ambiant anormal. Sous 30%, hallucinations client-side individuelles (portes qui n'existent pas, faux monstres — asymétrique, un joueur peut halluciner, pas les autres).
- **Stamina** : drainée par le sprint, pénalisée par le poids porté.
- **Poids d'inventaire** : les gros objets (loot de valeur, l'Ancre) se portent à deux mains → pas d'usage d'outil pendant ce temps.
- **Infection** (Partygoer) : transforme un joueur en menace passive-agressive pour l'équipe, sans le tuer immédiatement.

## 1.5 La Triade de Lumière (mécanique signature — conservée intégralement)
| Mode | Avantage | Coût |
|---|---|---|
| **Lampe standard** | Tu vois, tu peux travailler | Attire le Smiler et les entités photophiles |
| **Obscurité totale** | Invisible aux prédateurs lumineux | Sanity qui chute, pièges physiques invisibles |
| **Lumière UV** | Repousse/brûle les entités d'ombre | Batterie rare, se vide vite |

## 1.6 Arsenal scientifique (Gadgets — pas d'armes)
| Outil | Fonction | Contrainte |
|---|---|---|
| **Tazer / Micro-onde sonique** | Repousse une entité (knockback), n'inflige aucun dégât létal | Portée ~3m, surchauffe après 1 tir |
| **Flash Strobe** | Éblouit temporairement, révèle la géométrie cachée | Batterie limitée |
| **Spray Eau d'Amande** | Soigne la Sanity / calme une entité agressive | Consommable, aussi vendable = dilemme risk/reward |
| **Leurre Audio (Radio)** | Attire l'IA sonore ailleurs | Usage unique par leurre |
| **Scanner LIDAR** | Rend visibles les Dullers, la géométrie dans le noir absolu | Nécessaire dans certains biomes (Caves) |
| **Ancre de Réalité** | Zone sûre déployable (regen Sanity, anti-Shift local) | Très lourde, batterie ~3 min, coûteuse (2000 crédits) |
| **Résonateur Harmonique (no-clip)** | Traverse un mur — fuite d'urgence | 70% succès / 30% échec critique (mort ou téléportation aléatoire). Trouvable en loot uniquement, jamais achetable, usage unique |

## 1.7 Bestiaire (noms canoniques unifiés)
| Entité | Punit quoi | Comportement clé |
|---|---|---|
| **Smiler** | L'usage de la lumière | Charge si éclairé, se fige si observé dans le noir |
| **Skinwalker** | Le bruit / la dispersion de l'équipe | Enregistre le micro (buffer circulaire RAM), rejoue la voix pour isoler un joueur |
| **Hound** | Le mouvement bruyant / la fuite dos tourné | Chasse au bruit, se fige si regardé (mécanique "Ange Pleureur") |
| **Duller** | L'absence de LIDAR | Invisible à l'œil nu, visible seulement au scanner |
| **Clump** | L'inattention | Agrippe depuis un mur/trou, nécessite un coéquipier pour le QTE de libération |
| **Watcher** | La curiosité | Statique, draine la Sanity si on le fixe |
| **Jerry** | L'approche imprudente | Endoctrine un joueur (perte de contrôle), doit être secouru de force |
| **Partygoer (Infection)** | Le contact avec un allié infecté | Transforme un joueur en menace sociale passive |
| **Hydrolitis** | Le fait de rester dans l'eau | Entité liquide des Poolrooms |
| **Wretch** | Le bruit en général | Le "zombie" de base, sert de menace d'ambiance |

## 1.8 Biomes (7 niveaux)
1. **Niveau 0 — The Lobby** : murs jaunes, moquette, géométrie qui boucle.
2. **Niveau 1 — Habitable Zone** : industriel, béton, passerelles, brume.
3. **Niveau 2 — Pipe Dreams** : tunnels de maintenance, vapeur, chaleur.
4. **Niveau 4 — Abandoned Office** : bureaux vides, calme oppressant, forte drain de Sanity.
5. **Niveau 37 — Poolrooms** : carrelage, eau, son porté loin, ralentissement.
6. **Niveau 8 — Cave System** : noir total, LIDAR quasi obligatoire.
7. **Niveau ! — Run For Your Life** : couloir rouge infini, horde, sprint pur.

## 1.9 Détérioration de niveau & Liminal Shift
- Variable globale `CurrentStability` (100% → 0%) dans le GameState, répliquée.
  - **100-60%** : normal.
  - **60-20%** : lumières clignotantes, portes qui se verrouillent aléatoirement.
  - **20-0%** : Collapse — horde, dégâts environnementaux.
- **Liminal Shift** : le serveur ne modifie que les portes/tuiles qu'aucun joueur ne regarde/n'occupe (règle "Schrödinger"). Implémentation la plus sûre : changer l'état de portes logiques (ouvert/fermé/bloqué) plutôt que détruire des murs physiques.

## 1.10 Économie & Progression du Hub
- Le loot de faible valeur (cuivre, fusibles, laptops cassés) sert à faire évoluer le Hub, pas juste à être vendu.
- **États du Hub** :
  - **Stade 0** (taudis)
  - **Stade 1** (Stabilisation, lumière/lit)
  - **Stade 2** (Terminal avancé, débloque biomes difficiles)
  - **Stade 3** (Confort, buffs pré-run)
- Stocké dans `LiminalSaveGame` : `int32 HubLevel`, `TArray<FName> UnlockedUpgrades`.

## 1.11 Objectif final & Rejouabilité
- Construction progressive du **Portail Alpha** une fois le Hub maxé → mission finale "L'Évasion" (raid unique, très difficile).
- Réussite → badge de Prestige + avantage permanent en New Game+, reset propre au Niveau 0.

## 1.12 Types de missions
- **Maintenance** : fusibles, valves, redémarrage serveur.
- **Recherche** : scanner une entité, récupérer une VHS, prélever un échantillon.
- **Survie** : purger une zone, survivre à un blackout, exfiltrer un agent perdu.

## 1.13 Ton narratif
Froid et clinique dans les rapports du M.E.G. Fragmenté et désespéré dans les notes de survivants. Jamais de vocabulaire "gaming" (HP, spawn, aggro) dans les textes en jeu — parler d'"Intégrité Biologique", "Réponse Hostile", "Manifestation". Humour noir seulement, jamais léger.

## 1.14 Registre des classes C++
- `ALiminalSurvivor` / `AScavengerCharacter` : `ACharacter` — Sanity, Poids, Infection, Wifi
- `ALiminalEntity` : `ACharacter` — classe mère IA, enum `EMonsterType`
- `ALiminalGadget` / `ABaseTool` : `AActor` — base Lidar / Tazer / Leurre
- `ALiminalAnchor` / `ARealityAnchorTool` : `AActor` — Ancre de Réalité (safe zone)
- `ALiminalLevelGenerator` : `AActor` — PCG vertical + Liminal Shift
- `ALiminalShopTerminal` : `AActor` — achat au Hub
- `ALiminalGameMode` : timer extraction, Director (hordes)
- `ULiminalGameInstance` : persistance Hub/Inventaire entre maps
- `UVoiceMimicryComponent` : `UAudioCaptureComponent` — buffer circulaire vocal
- `FLootItem` (struct), `EEntityState` (enum)

---

# ARCHITECTURE TECHNIQUE & FONDATIONS (DÉTAILS SYSTÉMIQUES)

## 1. GAMEPLAY LOOP & ÉCONOMIE

**Rôle** : Employé jetable du M.E.G. (Major Explorer Group).
**Objectif** : Remplir un Quota de ressources (Eau d'Amande, Reliques, Ferraille, Données Topographiques) pour maintenir le Hub (Base Alpha).

### La Boucle de Gameplay Opérationnelle :

1. **HUB (Safe Zone — `ALiminalLobbyGameMode`)** :
   - Zone sûre exempte d'ennemis et de drain de sanité.
   - Banque de départ (250 crédits) et gestion de stock.
   - Terminal interactif (`ALiminalTerminalActor`) : achat d'outils d'escouade via catalogue `FTerminalStoreItem` (100 crédits par outil), sélection de biome parmi les 11 disponibles (`ServerSelectBiome`), et lancement synchronisé de l'incursion (`ServerLaunchIncursion`).
2. **INSERTION** :
   - Transition de niveau vers la carte d'incursion (`ALiminalGameMode` sur `Lvl_Loop` ou `Lvl_ProcGen`).
   - Spawn sécurisé des joueurs dans la première salle générée (`SpawnPlayerStarts`).
3. **SCAVENGE (Tension & Danger)** :
   - Exploration des salles et corridors instanciés (`ALiminalLevelGenerator`).
   - Localisation du butin physique (`ALootActor`).
   - Saisie physique à deux mains (`PhysicsHandleComponent`) : bloque immédiatement l'usage de tout outil.
   - Friction physique : chaque kilo porté ralentit le joueur (jusqu'à 35% de la vitesse de base à pleine charge de 60 kg) et augmente le coût d'endurance en sprint.
   - Vigilance sonore : la course et le déplacement lourd émettent du bruit répliqué pour les IA auditives (`MakeNoise`).
4. **EXTRACTION (Climax & Quota)** :
   - Ralliement de la zone d'extraction (`AExtractionZone`), systématiquement placée dans la salle la plus éloignée du point de spawn.
   - Compte à rebours de sécurisation (5 secondes) nécessitant le maintien des joueurs dans le volume.
   - Livraison automatique du loot porté au `UQuotaManager` : conversion en valeur de crédits.
   - Calcul du quota : la cible de base (100 crédits) doit être atteinte. En cas de déficit en fin de cycle, le manque est automatiquement accumulé sous forme de dette (`FQuotaLogic::ApplyFailure`).

### Objectifs à trois échelles (cohérence court/moyen/long terme)

| Type d'Objectif | Mécanique Associée | Motivation du Joueur | Référence |
|---|---|---|---|
| Court Terme | Collecte de Quota, Survie | Éviter la dette/mort, sécuriser le butin | Lethal Company, Deep Rock Galactic |
| Moyen Terme | Amélioration du Hub/Équipement | Achat d'outils au terminal, accès aux biomes profonds | Pacific Drive, Voidtrain |
| Long Terme | Cartographie Totale, Résolution Narrative | Percer le mystère des 11 biomes, stabilité dimensionnelle | Subnautica, Arc Raiders |

### Mécanique de Poids et Friction Physique
Pas d'inventaire abstrait sous forme de liste infinie. Les objets possèdent une masse réelle en kilogrammes :
- Porter un objet lourd (ex : Blindage en plomb de 35 kg ou Bobine de cuivre de 12 kg) mobilise les deux mains et neutralise les outils.
- La vitesse de marche est dynamiquement recalculée (`WalkSpeedAtMaxWeightFactor = 0.35f`).
- L'endurance s'épuise 1,5× plus vite en sprint chargé.
- Tout lâcher précipité au sol génère un son de collision (`CollisionSound`) capté par l'IA ouïe à plus de 15 mètres.

---

## 2. MÉCANIQUES "NO-GUNS" (SYSTÈME D'OUTILS)

Le combat frontal létal est totalement proscrit. Le joueur est équipé d'instruments analogiques de mesure, de temporisation et de diversion.

**Classe de Base C++** : `ABaseTool` (hérite de `AActor`). Gère la batterie répliquée, le refroidissement, le cooldown et les RPC serveur (`ServerUseTool`).

### A. Outils Opérationnels (Phase 1 — Entièrement Implémentés en C++)

#### Outil 1 : Le Flash Strobe (`AFlashStrobeTool`)
- **Fonction** : Décharge un flash lumineux conique puissant (portée 8m, cône 45°).
- **Effet** : Éblouit et étourdit immédiatement les entités (`ApplyStun` 3.0s), particulièrement dévastateur contre le Smiler et la Deathmoth.
- **Contrainte** : Consomme 25% de la batterie par utilisation. Régénération passive lente.

#### Outil 2 : Le Micro-Onde Sonique (`ASonicMicrowaveTool`)
- **Fonction** : Émet une onde acoustique de choc directionnelle à courte portée (4,5m).
- **Effet** : Applique une impulsion physique violente (120 000 unités) repoussant l'entité et un étourdissement de 2,5s.
- **Contrainte** : Surchauffe immédiate après chaque tir (cooldown de refroidissement).

#### Outil 3 : Le Spray d'Eau d'Amande (`AAlmondWaterSprayTool`)
- **Fonction** : Pulvérise un aérosol apaisant sur 3,5 mètres.
- **Effet** : Calme les monstres enragés comme le Wretch (`ApplyCalm` 4.0s) et restaure la Sanité du porteur et de ses coéquipiers proches.
- **Contrainte** : Réserve de fluide limitée, dispersion lente.

#### Outil 4 : Le Leurre Audio (`AAudioDecoyTool`)
- **Fonction** : Radio de transmission analogique déployable au sol.
- **Effet** : Émet un signal sonore périodique puissant pendant 15 secondes, détournant l'attention des entités auditives (`AISense_Hearing` / Hounds).
- **Contrainte** : Usage temporaire, attire tous les prédateurs de la zone vers sa position.

### B. Gadgets Scientifiques (Phase 2 — Entièrement Implémentés en C++)

#### Outil 5 : Scanner LIDAR (`ALidarScannerTool`)
- **Fonction** : Projette des impulsions coniques de scan volumétrique révélant la topologie dans le noir complet et perçant le camouflage optique des entités invisibles (Dullers).
- **Contrainte** : Consomme 12% de batterie par impulsion de scan.

#### Outil 6 : Analyseur de Signal (`ASignalAnalyzerTool`)
- **Fonction** : Console portable diégétique avec spectrographe radio pour pister les anomalies et le butin à travers les cloisons sur un rayon de 25 mètres.
- **Contrainte** : Consomme 8% de batterie par balayage.

#### Outil 7 : Ancre de Réalité (`ARealityAnchorTool`)
- **Fonction** : Dispositif lourd (30 kg) déployable au sol créant un champ de stabilisation dimensionnelle (rayon 8,5m) qui bloque tout drain de sanité et neutralise les hallucinations pour les joueurs à proximité.
- **Contrainte** : Poids extrême (30 kg), durée limitée par déploiement (60s).

#### Outil 8 : Câble de Liaison / Tether (`ATetherTool`)
- **Fonction** : Câble de liaison physique reliant deux coéquipiers pour prévenir l'isolement et la dispersion dans les zones de visibilité nulle.
- **Contrainte** : Longueur maximale (15m), se rompt en cas d'étirement excessif.

#### Outil 9 : Craie & Balises (`AChalkMarkerTool`)
- **Fonction** : Marquage de repérage directionnel au sol pour naviguer sans boussole dans les dédales non-euclidiens.
- **Contrainte** : Réserve limitée à 16 marques.

---

## 3. INTELLIGENCE ARTIFICIELLE (AI PERCEPTION & BESTIAIRE COMPLET)

L'IA réagit aux stimuli sensoriels réels via `UAIPerceptionComponent` natif UE :
- `AISense_Hearing` : Réception des bruits de pas (`MakeNoise` émis lors de la marche/course par `AScavengerCharacter`), des chutes d'objets, et des leurres audio.
- `AISense_Sight` : Détection visuelle directe et réaction aux cônes de torche.

### Les 10 Entités Opérationnelles (Implémentées en C++) :

1. **Smiler (`ALiminalEntity_Smiler`)** :
   - *Comportement* : Rôdeur photophobe invisible dans l'obscurité (seuls ses yeux et son rictus luisent). Reste passif si ignoré dans l'ombre.
   - *Stimulus* : Charge frénétiquement si le joueur le balaye directement avec sa lampe torche.
   - *Contre-mesure* : Couper la lampe, fixer son regard sans bouger pour le figer, ou déclencher le Flash Strobe.

2. **Hound (`LiminalAIController`)** :
   - *Comportement* : Prédateur quadrupède aveugle doté d'une ouïe absolue.
   - *Stimulus* : Traque toute émission sonore dépassant le seuil de discrétion.
   - *Contre-mesure* : Déplacement accroupi, utilisation du Leurre Audio pour orienter sa charge.

3. **Clump (`ALiminalEntity_Clump`)** :
   - *Comportement* : Amas tentaculaire statique dissimulé dans les recoins et corridors étroits.
   - *Stimulus* : Déclenchement au contact physique direct ; capture et immobilise la victime.
   - *Contre-mesure* : Contournement préventif ou projection d'onde de choc au Micro-Onde Sonique pour briser son étreinte.

4. **Watcher (`ALiminalEntity_Watcher`)** :
   - *Comportement* : Silhouette humanoïde immobile dans les grandes salles abandonnées.
   - *Stimulus* : Ne poursuit pas physiquement, mais draine agressivement la Sanité du joueur tant qu'il reste dans son champ de vision.
   - *Contre-mesure* : Rompre immédiatement le contact visuel, quitter la pièce, spray d'eau d'amande.

5. **Wretch (`ALiminalEntity_Wretch`)** :
   - *Comportement* : Humanoïde décharné errant dans les couloirs.
   - *Stimulus* : Attiré par le bruit d'impact et de course, attaque rapide au corps-à-corps.
   - *Contre-mesure* : Calmé temporairement par le Spray d'Eau d'Amande (`ApplyCalm`), repoussé par impulsion sonique.

6. **Deathmoth (`ALiminalEntity_Deathmoth`)** :
   - *Comportement* : Phalène géante volante patrouillant en hauteur.
   - *Stimulus* : Agression ciblée sur les sources de chaleur et les joueurs isolés.
   - *Contre-mesure* : Désorientation par le stroboscope, dispersion de l'escouade.

7. **Skinwalker (`ALiminalEntity_Skinwalker`)** :
   - *Comportement* : Traqueur prédateur qui détecte l'isolement des joueurs et utilise le composant de mimétisme vocal (`UVoiceMimicryComponent`) pour rejouer des extraits de voix d'alliés à distance afin de les attirer dans une embuscade.
   - *Contre-mesure* : Rester groupé, utiliser des mots de passe vocaux d'escouade.

8. **Partygoer (`ALiminalEntity_Partygoer`)** :
   - *Comportement* : Entité très véloce transmettant une infection sociale asymétrique au moindre contact physique (`TryInfectScavenger`), transformant le joueur en traître.
   - *Contre-mesure* : Éviter tout contact rapproché, quarantaine immédiate, tir de barrage répulsif au Micro-Onde Sonique.

9. **Duller (`ALiminalEntity_Duller`)** :
   - *Comportement* : Rôdeur totalement invisible à l'œil nu (`bIsRevealed = false`), traquant silencieusement les explorateurs.
   - *Contre-mesure* : Détecté et révélé temporairement par les impulsions du Scanner LIDAR (`ALidarScannerTool`) ou le Flash Strobe.

10. **Jerry (`ALiminalEntity_Jerry`)** :
    - *Comportement* : Oiseau psionique imposant un verrouillage visuel télépathique qui hypnotise et paralyse la victime sur place tout en drainant sa sanité.
    - *Contre-mesure* : Un allié doit briser la transe en infligeant des dégâts à Jerry ou en repoussant la victime au Micro-Onde Sonique..

---

## 4. ASPECTS TECHNIQUES UNREAL ENGINE 5

### A. Gestion Physique (Chaos Physics)
- Le Loot n'est pas juste une icône. Les gros objets sont des `PhysicsActors`.
- Le joueur doit utiliser un `PhysicsHandle` pour porter les objets lourds (comme Amnesia ou Half-Life).
- Porter un objet lourd ralentit le joueur et fait du bruit (collisions).

### B. Génération Procédurale (PCG Framework) + Géométrie Non-Euclidienne
- Utiliser le PCG Framework pour générer les couloirs infinis (Murs, Sols, Plafonds) au runtime.
- Système de "Tuiles" (Tiles) pour connecter les salles.
- **World Partition / Streaming** : charger/décharger des chunks de niveau autour du joueur de façon transparente — permet des niveaux théoriquement infinis et des reconfigurations dynamiques (le joueur se retourne, le couloir a changé).
- **Portals + Stencil Buffer** (technique) : plutôt que téléporter le joueur (saccades), rendre la vue de "sortie" du portail sur la surface d'"entrée", réaligner la caméra instantanément à la traversée. Permet des couloirs qui se replient sur eux-mêmes, des pièces plus grandes à l'intérieur qu'à l'extérieur (effet Tardis), ou une porte "Sortie" qui ramène au début sauf si on la traverse à reculons.
- **Verticalité** (inspiration Blame!) : cages d'escalier sans fin, puits d'ascenseurs ouverts sur le vide, mécaniques d'escalade/grappin — complète la claustrophobie horizontale par de l'acrophobie.

### C. Audio Immersif (Metasounds)
- Système de réverbération convolutionnelle (le son change selon la taille de la pièce).
- Voice Chat In-Game obligatoire : Le micro du joueur est une source de bruit pour l'IA (`MakeNoise`). Si tu cries IRL, le monstre t'entend In-Game.
- Voice Chat Buffer exploité par le Skin-Stealer (voir section 3).

### D. Système de Sanité (Post-Process) — "Sanity System 2.0"
- Pas de barre de vie/sanité classique visible. Score de "Stabilité Réelle" caché, qui baisse en présence d'anomalies, dans le noir, ou face à des géométries impossibles.
- **Symptômes visuels** : distorsion de lentille (aberration chromatique), couloirs qui semblent s'allonger (effet Dolly Zoom), portes qui disparaissent hors du champ de vision, murs qui respirent, géométrie non-euclidienne simulée via `WorldPositionOffset` dans le shader.
- **Symptômes auditifs** : bruits de pas fantômes, respiration désynchronisée.
- **Symptômes systémiques (UI diégétique cassée)** : fausses notifications d'interface ("Connexion perdue", "Inventaire plein" alors qu'il est vide) — le jeu fait douter le joueur de son propre matériel plutôt que de son personnage.

---

## 5. LOGIQUE DE TRANSITION ENTRE NIVEAUX

Le déplacement physique seul ne justifie pas un changement de niveau dans un espace non-euclidien — il faut une logique systémique ancrée dans la fiction (technobabble scientifique cohérent).

### Modèle retenu : Seuil Énergétique + Sécurité (hybride GTFO / Pacific Drive)
- Le joueur localise des **Nœuds de Stabilité** (Stability Anchors) au Niveau 0 via le scanner.
- Extraire un Nœud déstabilise localement la zone (déclenche des événements hostiles) — risque immédiat pour récompense.
- Pour passer au Niveau 1, il faut soit :
  - **Clé de Sécurité** : trouvée/piratée dans une Zone Admin dangereuse (façon GTFO, "Bulkhead Controller") — chasse au trésor structurée, pas de chute aléatoire (noclip) dans le niveau suivant.
  - **Surcharge Énergétique** : utiliser l'énergie collectée sur un "Seuil" qui vibre, ouvrant une brèche — mais consomme toute la réserve, rendant le retour impossible en cas d'échec (Push your luck).
- Le véhicule/équipement doit être amélioré au Hub (blindage, isolation) avant de pouvoir survivre à la toxicité/instabilité du niveau suivant — barrière "douce" (Soft Gate) cohérente avec la survie plutôt qu'un simple mur de niveau arbitraire.

### Véhicule de transition : le V.E.L. (Véhicule d'Exploration Liminale)
Chariot électrique blindé servant de Hub mobile. Sert de sas de décompression (tension avant l'ouverture des portes), transporte le joueur et absorbe une partie du danger environnemental (radiations/instabilité), nécessite des upgrades débloqués au Hub pour accéder aux niveaux profonds.

---

## 6. LE HUB — L'INSTITUT ASYNC

Le Hub est l'unique ancre de réalité : géométrie euclidienne stable, contrairement aux niveaux.

### Zones
- **Zone de Planification** : carte holographique des secteurs découverts, sélection de mission.
- **Zone de Fabrication** : atelier transformant les ressources brutes (ferraille, fluide) en équipement.
- **Zone de Vie** : cafétéria, dortoirs — humanise les personnages, crée un attachement émotionnel au lieu sûr.

### Le Hub comme narrateur environnemental
Le Hub évolue visuellement avec la progression : campement de fortune (bâches plastique, lampes de chantier) → murs en béton, vrais laboratoires, PNJ qui s'installent. C'est la récompense long-terme la plus lisible : voir l'ordre triompher visuellement du chaos.

---

## 7. PROGRESSION

### A. Arbre Technologique (Hard Gating)
Le joueur débloque des plans (Blueprints) via une Station de Fabrication. Exemple de boucle : Explorer Zone Facile → Récupérer Ressources → Crafter équipement (ex : Lampe Iode Haute Intensité) → Explorer Zone Difficile → Trouver Clé Niveau Suivant → Accéder au niveau. Crée des mini-objectifs clairs sans besoin de narration complexe.

### B. Économie de l'Information (Soft Gating)
Des Dossiers de Renseignement (Intel) cryptés, ramenés et analysés au Hub, débloquent des informations sur les dangers des niveaux suivants (ex : savoir qu'un niveau est peuplé d'entités sensibles au son permet d'acheter des bottes silencieuses en amont). Progression par la connaissance plutôt que par la statistique pure — cohérent avec le rôle "scientifique" du joueur, pas "soldat qui level up".

### C. Gear-Gating plutôt que Skill-Gating
L'argent/les ressources achètent de meilleurs équipements scientifiques (scanners, combinaisons, ancres) permettant d'accéder à des niveaux plus profonds. Pas de montée en compétence du personnage — le joueur reste vulnérable tout du long, c'est la mécanique centrale de la tension.

### Matrice de progression indicative (Les 11 Biomes C++)

| Niveau | Environnement | Ressource Clé | Amélioration Débloquée | Dangers / Entités Spécifiques |
|---|---|---|---|---|
| Niveau 0 (Yellow Lobby) | Moquette jaune, 2000 lux | Ferraille, Données Brutes | Sac à dos renforcé, Batterie T1 | Hound, Smiler |
| Niveau 1 (Habitable Zone) | Béton, 1600 lux | Composants Électroniques | Lampe Torche focalisée | Hound, Clump |
| Niveau 2 (Pipe Dreams) | Tunnels vapeur, 1400 lux | Tuyaux cuivre, Fluide | Bottes insonorisées | Hound, Clump, Wretch |
| Niveau 3 (Electrical Station) | Haute tension, 1750 lux | Transformateurs, Relais | Résistance aux décharges | Hound, Smiler |
| Niveau 4 (Abandoned Office) | Bureaux vides, 2200 lux | Cassettes de données | Filtre mental anti-panique | Hound, Watcher, Smiler |
| Niveau 6 (Lights Out) | Noir absolu, 20 lux | Cristaux non-euclidiens | Vision nocturne T1 | Hound, Wretch, Smiler |
| Niveau 8 (Cave System) | Roche accidentée, 700 lux | Minéraux instables | Chaussures d'adhérence | Hound, Deathmoth, Clump |
| Niveau 9 (Dark Suburbs) | Faubourg nocturne, 1000 lux | Équipement civil abandonné | Radio longue portée | Hound, Wretch |
| Niveau 10 (Wheat Fields) | Champs dorés, 1900 lux | Matière organique pure | Combinaison légère | Hound, Deathmoth |
| Niveau 37 (Poolrooms) | Carrelage blanc, 1800 lux | Eau déminéralisée | Déplacement aquatique | Hound, Smiler |
| Niveau ! (Run For Your Life) | Couloir d'urgence, 3500 lux | Données d'évasion | Équipement d'endurance T3 | Horde (Smiler, Wretch, Hound) |

### Typologie de missions (variété d'objectifs au Hub)

| Type | Objectif Ludique | Récompense Méta |
|---|---|---|
| Reconnaissance | Explorer 5 Salles et cartographier | Dévoilement du secteur |
| Récupération | Rapporter 100 crédits de valeur | Validation du Quota de cycle |
| Stabilisation | Déployer une balise de signal | Déblocage de biomes profonds |
| Sauvetage | Évacuer le loot d'un coéquipier tombé | Économie de pénalité de dette |

---

## 8. UNIVERS ET LORE — "Pas trop de lore, mais un monde"

Rupture volontaire avec le lore Backrooms sur-explicité (dates, noms d'explorateurs, taxonomie précise des monstres) qui a tué le mystère du genre.

- **Le monde n'explique rien** : machines qui bourdonnent, tuyaux qui fuient, traces de campements sans savoir qui/pourquoi. Le joueur spécule — l'histoire vécue dans l'instant prime sur le texte lu dans une note.
- **Esthétique de l'Anemoia** (nostalgie d'un temps jamais vécu) : lieux familiers vidés de sens — centres commerciaux 90s vides (mallsoft), parcs aquatiques sans eau, hôtels d'aéroport infinis. Résonne plus profondément qu'un monstre griffu.
- **Objectif final ouvert** (à trancher en développement) : soit la stabilisation totale du secteur (fin "positive"), soit un effondrement dimensionnel dû à l'hubris technologique du M.E.G. (fin "catastrophe", justifiant une Wipe narrative en cas de suite/saison).

---

## 9. ASPECTS TECHNIQUES SPÉCIFIQUES — ITEMS

### Inventaire "Tetris" (grille, pas liste infinie)
Système type Resident Evil / Deus Ex où taille et forme des objets comptent physiquement dans l'espace de stockage.

### Exemples d'objets et contraintes associées

| Objet | Fonction | Contrainte | Impact Psychologique |
|---|---|---|---|
| Scanner LIDAR | Révèle géométrie dans le noir | Batterie faible, champ réduit | Peur du hors-champ |
| Ancre de Réalité | Crée Safe Zone temporaire | Très lourd, empêche de courir | Dépendance aux coéquipiers |
| Détecteur EMF | Alerte proximité entités | Bip sonore fort qui attire les entités | Risque vs Information |
| Craie/Marqueur | Trace le chemin | Ressource limitée, marques effaçables | Doute sur sa mémoire |
| Câble de Connexion | Relie deux joueurs (tether) | Limite distance de mouvement | Claustrophobie sociale |

### Maintenance comme mécanique de survie
Inspiré Pacific Drive : équipement vital (combinaison HAZMAT, générateur portable) nécessite une maintenance manuelle et diégétique — ex : filtre à air encrassé → vision trouble/hallucinations → le joueur doit physiquement le retirer/secouer/remplacer, souvent dans le noir ou en pleine traque.

---

## 10. STRUCTURE DES DONNÉES & ARCHITECTURE TECHNIQUE (C++)

Le projet repose sur une architecture modulaire stricte, server-authoritative et testable :

- **`UItemData`** (PrimaryDataAsset) : Données d'objets (Nom, Poids, Mesh, Valeur crédits, Son de collision).
- **`FLootItem`** (Struct FTableRowBase) : Entrées de la DataTable `DT_LootItems` pour le butin physique.
- **`FQuotaState` & `FQuotaLogic`** : Moteur mathématique pur du quota et du report de dette (100% testable hors moteur).
- **`UQuotaManager`** (GameInstanceSubsystem) : Suivi en temps réel de la livraison et de la dette globale.
- **`FLiminalSaveData` & `ULiminalSaveGame`** : Sauvegarde disque des crédits bancaires (250 au départ), du cycle actif et des biomes débloqués.
- **`ULiminalGameInstance`** : Gestionnaire de session, host/join, sélection de biome et transitions Hub ↔ Mission.
- **`AScavengerCharacter`** : Personnage récupérateur avec `PhysicsHandleComponent`, gestion de santé, endurance, charge physique, lampe frontale et cycling d'outils.
- **`ABaseTool` & Outils** : `AFlashStrobeTool`, `ASonicMicrowaveTool`, `AAlmondWaterSprayTool`, `AAudioDecoyTool`.
- **`ALootActor`** : Acteur physique lootable doté de masse, collision, réplication réseau et surbrillance au regard.
- **`AExtractionZone`** : Volume d'extraction déclenchant le compte à rebours de sécurisation et la livraison du loot.
- **`ALiminalTerminalActor`** : Terminal marchand interactif du Hub pour la sélection de biome et l'achat d'équipement.
- **`ALiminalEntity` & Sous-Classes** : `ALiminalAIController`, Smiler, Watcher, Clump, Deathmoth, Wretch.
- **`FLiminalLayoutBuilder` & `ALiminalLevelGenerator`** : Génération procédurale de labyrinthes déterministes et instanciation dynamique des 11 biomes.
- **`UScavengerHUDWidget`** : HUD analogique UMG (santé, sanité, stamina, poids, crédits, outil actif).
- **`ALiminalDeathScreenHUD`** : Écran de mort diégétique CRT monochrome vert (1998) avec rapport K.I.A., cause de décès, crédits perdus et passage au mode spectateur CCTV.
- **`ALiminalPauseMenuComponent`** : Menu de pause diégétique dans le terminal avec réglages de sensibilité, volume audio et abandon de mission.
- **`ALiminalBreakerActor` & `ALiminalKeypadActor`** : Puzzles environnementaux (disjoncteurs muraux, digicodes de sécurité) reliés procéduralement à l'extraction.
- **`AExtractionZone`** : Volume d'extraction conditionnel (requiert alimentation électrique et/ou code d'accès) avec état lumineux dynamique vert/rouge.
- **`AScavengerCharacter`** (Survie Coopérative & Feedback) : État K.O./Agonisant (`bIsDowned`), minuterie de 45s, ramper au sol, réanimation d'équipier, largage physique du loot, flash de dégâts crimson et secousse de caméra.
- **`MegReclamationTests`** : Suite complète de 25 tests d'automatisation native UE validant la totalité du gameplay (100% de succès, exit code 0).

---

## 💡 Analyse Stratégique — Facteurs de Différenciation

### Le Facteur "Clip Twitch/TikTok" (Viralité)
- La mécanique de Mimétisme (`ALiminalEntity_Skinwalker` et `UVoiceMimicryComponent`) est de l'or pur pour les streamers. Entendre son ami crier "Aide-moi!" pour réaliser ensuite qu'il est mort depuis 5 minutes crée des moments de terreur inoubliables.
- La physique des objets (porter un gros moteur lourd qui cogne contre les murs en fuyant un monstre) crée du "slapstick horror" (mélange de peur et de rire) très populaire.

### La Tension "Risque vs Récompense"
- Devoir tenir les objets à deux mains (et donc lâcher lampe torche ou détecteur) force la coopération. Un joueur porte, l'autre éclaire et protège.
- Si le porteur tombe au combat, l'état K.O. permet aux coéquipiers d'administrer une dose d'adrénaline (`AAdrenalineInjectorTool`) ou de récupérer le butin largué au sol pour sauver le quota.

### L'Atmosphère "Found Footage" (Visuel)
- Caméra "Bodycam" avec léger effet fish-eye et stabilisation logicielle — rendu ultra-réaliste façon Unrecord, dans les Backrooms. OSD VHS avec timecode diégétique, alertes de sanité corrompues et clignotement d'interférences près des entités.

### Tableau comparatif vs concurrence directe

| Caractéristique | Escape the Backrooms (existant) | M.E.G. : Reclamation (ce projet) |
|---|---|---|
| Objectif | Linéaire (atteindre la fin du niveau) | Non-linéaire (extraction de ressources, cartographie, quotas) |
| Monstres | Scriptés, chasse active | Systémiques, mimétisme vocal, hypnose Jerry, menace auditive |
| Level Design | Statique, couloirs plats | Procédural déterministe (11 biomes), puzzles de disjoncteur/digicode |
| Coopération | Optionnelle | Cruciale (réanimation d'équipier, objets lourds, rôles asymétriques) |
| Ambiance | Jump scares fréquents | Tension atmosphérique lente ("slow burn"), dégradation de sanité 2.0 |

---

## Roadmap de développement (statut d'avancement réel)

1. [x] `UItemData` + `UQuotaManager` + `FQuotaLogic` (fondations données, quota et dette)
2. [x] `ABaseTool` + les 4 outils de base (Flash Strobe, Micro-Onde Sonique, Spray Eau d'Amande, Leurre Audio)
3. [x] `AScavengerCharacter` (Endurance, friction du poids, sanité, santé, lampe, cycling d'outils)
4. [x] IA : `AIPerceptionComponent` + entités complètes (Smiler, Hound, Clump, Watcher, Deathmoth, Wretch, Skinwalker, Partygoer, Duller, Jerry)
5. [x] Chaos Physics + `PhysicsHandleComponent` + `ALootActor` pour le portage physique d'objets lourds
6. [x] Boucle d'extraction jouable (`ALiminalGameMode`, `ALiminalLobbyGameMode`, `AExtractionZone`, `ALiminalTerminalActor`, map `Lvl_Loop`)
7. [x] Génération procédurale (`FLiminalLayoutBuilder`, `ALiminalLevelGenerator` sur 11 biomes avec éclairage dynamique et scaling jusqu'à 64x64)
8. [x] Interface utilisateur (`UScavengerHUDWidget` UMG analogique + `ALiminalScavengerHUD` VHS OSD 1998)
9. [x] Persistance & Sauvegarde (`FLiminalSaveData`, `ULiminalSaveGame`, `ULiminalGameInstance`)
10. [x] Suite de tests automatisés native UE (25 tests `MegReclamationTests` validés avec succès, exit code 0)
11. [x] Validation multijoueur & Survie Coopérative (État K.O. Agonisant, ramper au sol, réanimation d'équipier `ServerRevivePlayer`, largage physique du loot, Spectateur CCTV `ALiminalSpectatorPawn`)
12. [x] Audio & Metasounds (Atténuation sonore spatialisée, bruits de pas dynamiques `ULiminalFootstepComponent`, bruits de pas fantômes en basse sanité, alertes sonores de disjoncteur)
13. [x] Système de sanité avancé 2.0 (Post-process vignette, aberrations chromatiques, distorsion FOV, fausses alertes d'anomalie de capteur diégétiques)
14. [x] Systèmes avancés de Phase 2 & Puzzles (Skinwalker avec buffer de mimétisme vocal, hypnose de Jerry, infection Partygoer, outils LIDAR/Signal/Ancre/Tether/Craie/Adrénaline, disjoncteurs et digicodes procéduraux reliant l'extraction)
