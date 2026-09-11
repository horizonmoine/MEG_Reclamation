# Bible de Level Design — M.E.G. : Reclamation

Référence pour `procgen_designer`, `bestiary_ai_engineer` et les missions 07 à 09. Complète `GDD.md` et `MASTER_CONTEXT.md` sans les remplacer. Unités : centimètres UE (1 m = 100), secondes.

## 1. Contrat de zone (implémenté : `ULiminalZoneRulesSubsystem`)
| Lieu | Phase `EMissionPhase` | Pression sanité | Entités | Loot | Timer |
|---|---|---|---|---|---|
| Menu, Lobby | aucune (`ALiminalGameState` absent) | Non | Non | Non | Non |
| Hub Base Alpha | `Hub` | **Non** + restauration 2/s dans `ALiminalSafeZoneVolume` | Non | Non | Non |
| Sas B.R.C. (non scellé) | `Airlock` | Non | Non | Non | Non |
| Sas scellé -> biome | `Incursion` | Oui (hors zones sûres) | Oui | Oui | 480 s démarré |
| Effondrement | `Collapsing` (≥ 80 % du timer) | Oui x1.5 | Hordes | Oui | Fin à 0 |
| Extraction / Échec | `Extracted` / `Failed` | Non | Non | Non | Arrêté |

Règle d'or : **aucun système ne décide seul s'il est en danger** ; tous interrogent `ULiminalZoneRulesSubsystem`.

## 2. Hub Base Alpha (`Lvl_Hub_BaseAlpha`)
Espace 40 x 30 m sur un niveau, béton brut + néons blancs 4000 K stables (jamais de flicker : contraste avec les biomes). Un `ALiminalSafeZoneVolume` englobe toute la map (extent 2500 x 2000 x 500).

| Salle | Dimensions | Actors C++ | Fonction |
|---|---|---|---|
| Dortoir (spawn) | 8 x 6 m | `PlayerStart` x4, casiers | Spawn, lecture du rapport de run précédent (affiche `FLiminalRunSummary`) |
| Comptoir logistique | 10 x 8 m | `ALiminalTerminalActor` (boutique, sélection biome, lancement) | Achat outils, quota `Q(k,N)`, dette |
| Infirmerie | 6 x 6 m | `ALiminalSafeZoneVolume` dédié `SanityRestorePerSecond = 8` | Récupération rapide, soin |
| Atelier | 8 x 6 m | `ULiminalCraftingComponent`, `ALiminalFuseBoxActor` (tutoriel) | Craft, recharge batteries |
| Salle de briefing | 8 x 6 m | Tableau des 11 biomes (état déverrouillé via `UnlockedBiomes`) | Choix collectif, lore |
| Sas B.R.C. | 6 x 4 m + console | `ALiminalAirlockActor`, console CLI (`SWITCH`, `RADAR`, `DOOR`…) | Point de départ ; scellement = tous `Alive` dans le volume + `bIsReady` |
| Couloir de retour | 4 x 12 m | `AExtractionZone` (côté biome) débouche ici | Debrief, crédit banque |

Flux : Dortoir -> Comptoir -> (Infirmerie/Atelier optionnels) -> Briefing -> Sas. Distance spawn -> sas : 35 m pour laisser le temps d'équiper. Aucun tutoriel textuel bloquant : signalétique diégétique + `ToggleFieldManual`.

## 3. Boucle d'une incursion (480 s) : courbe de pacing
| Fenêtre | Timer | Directeur (`ALiminalEntityDirector`) | Lumière/Audio | Objectif joueur |
|---|---|---|---|---|
| Insertion | 0-90 s | 0 entité active, 1 Watcher passif loin | Néons stables, drone bas | Repérer, marquer (craie), premier loot léger |
| Tension | 90-300 s | 1 traqueur (Hound ou Smiler selon biome) + 1 statique | Flicker 60 Hz local aux stimuli | Loot lourd, gestion poids, premiers choix de repli |
| Escalade | 300-384 s | +1 entité par joueur vivant, Skinwalker autorisé | Blackouts 45 s (`TriggerBlackout`) | Décider : extraire ou pousser |
| Effondrement | 384-480 s (`Collapsing`) | Hordes, stun réduit de 50 %, spawns à 30 m des joueurs | Néons meurent par zones, sirène diégétique | Course vers `AExtractionZone` |
| 0 s | `Failed` | Tout joueur non extrait -> `Dead`, cadavre -80 BR | Blanc puis noir | Debrief |

Sanité moyenne de l'équipe < 40 % : le Directeur retarde le prochain spawn de 20 s (éviter la spirale). > 80 % : il l'avance de 15 s.

## 4. Règles de génération procédurale (`LiminalModularDungeonGenerator`, `LiminalLayoutLibrary`)
1. Cellule WFC 6 x 6 x 4 m (GDD). Grille 12 x 12 cellules minimum, 20 x 20 maximum (N=4).
2. Seed unique = `ALiminalGameState::MissionSeed` (`FRandomStream`). Aucune autre source d'aléa dans la génération.
3. Spawn et extraction : distance géodésique ≥ 80 m ET ≥ 3 changements de direction. Extraction jamais visible depuis le spawn.
4. Connexité : A* sur la grille, 100 % des cellules loot atteignables ; culs-de-sac ≤ 15 % des couloirs et jamais > 2 cellules de profondeur.
5. Boucles : ≥ 2 cycles indépendants (fuite possible sans demi-tour). Portes verrouillées (`ALiminalDoorActor` + `ALiminalKeyItemActor`/`ALiminalKeypadActor`) uniquement sur des raccourcis, jamais sur le chemin critique.
6. Salles spéciales par run : 1 disjoncteur (`ALiminalBreakerActor`) contrôlant 40 % des néons, 1 puzzle vannes ou fusibles, 2 à 4 cachettes (`ALiminalHidingSpot`), 1 conduit (`ALiminalVentActor`) reliant deux boucles, 0 à 1 zone sûre fixe (`ALiminalSafeZoneVolume`, `SanityRestorePerSecond = 1`, extent 3 x 3 m) hors chemin critique.
7. Densité de loot : valeur totale générée = 1.6 x `Q(k,N)` (le quota est atteignable en laissant 40 % sur place). 60 % léger (≤ 5 kg), 30 % lourd deux mains, 10 % artefact rare.
8. ISM obligatoires pour dalles, cloisons, néons. Aucun mesh de couloir répliqué : les clients regénèrent depuis la seed.
9. Validation automatique (`MegProcGenSafetyTests`) : 100 seeds, 0 échec sur les règles 3 à 5.

## 5. Les 11 biomes
Colonnes : palette/lumière, tuiles dominantes, roster (les 9 entités : Smiler, Hound, Duller, Clump, Deathmoth, Skinwalker, Partygoer, Watcher, Wretch ; Jerry = événement rare), modificateur de sanité, tier de loot, danger environnemental, cycle de déverrouillage `k`.

| # | Biome | Palette & lumière | Tuiles | Roster | Sanité | Loot | Danger | k |
|---|---|---|---|---|---|---|---|---|
| 1 | Level 0 : Tutorial Yellow | Moquette jaune, papier peint humide, néons 60 Hz | Couloirs 3 m, pièces vides, aucune porte | Smiler (1), Watcher | x1.0 | T1 | Aucun | 1 |
| 2 | Level 1 : Habitable Zone | Béton gris, parking, flaques, lampes sodium | Halls 12 m, piliers, rampes | Hound, Smiler, Wretch | x1.0 | T1-T2 | Flaques (bruit x2) | 1 |
| 3 | Level 2 : Pipe Dreams | Tunnels de maintenance, vapeur, rouge d'urgence | Couloirs 2.5 m, échelles, vannes | Hound, Duller, Clump | x1.2 | T2 | Vapeur (dégâts 5/s), chaleur | 2 |
| 4 | Level 3 : Electrical Station | Salles machines, arcs électriques, stroboscopes | Grandes salles + passerelles | Deathmoth, Smiler | x1.1 | T2 | Sols électrifiés (disjoncteur) | 2 |
| 5 | Level 4 : Abandoned Office | Open space, néons froids, moquette | Bureaux, cloisons basses, salles de réunion | Skinwalker, Watcher, Partygoer (rare) | x1.3 | T2-T3 | Isolement (drain x1.5 si seul) | 3 |
| 6 | Level 5 : Terror Hotel | Bois sombre, tapis rouge, appliques chaudes | Couloirs 2 m, chambres, ascenseur | Wretch, Skinwalker, Jerry | x1.4 | T3 | Portes qui se referment (30 s) | 3 |
| 7 | Level 6 : Lights Out | Noir total, seule la lampe frontale | Labyrinthe 2 m, murs lisses | Smiler (x3), Hound | x1.6 (obscurité permanente) | T3 | Aucune lumière fixe | 4 |
| 8 | Level 7 : Thalassophobia | Eau noire jusqu'aux genoux, plafond bas | Bassins, pontons | Clump, Duller | x1.3 | T2 | Nage lente, bruit x3 | 4 |
| 9 | Level 9 : Suburbs | Nuit, lampadaires, maisons vides | Rues 8 m, intérieurs | Partygoer, Skinwalker | x1.2 | T3 | Extérieur : portée Hound x2 | 5 |
| 10 | Level 37 : Poolrooms | Carrelage blanc, eau turquoise, lumière diffuse | Bassins, arches, escaliers | Watcher, Deathmoth | x0.8 (apaisant) puis x1.5 sous 40 % | T3 | Noyade si sanité < 20 % | 5 |
| 11 | Level 188 : Windows | Lucarnes infinies, lumière grise, silence | Couloirs de verre, reflets | Skinwalker (x2), Watcher | x1.5 | T4 | Reflets = hallucinations x2 | 6 |

Progression : les biomes `k` ≤ cycle courant sont sélectionnables au Terminal. Tier loot T1 = 20-60 BR/objet, T4 = 150-400 BR.

## 6. Difficulté par nombre de joueurs `N`
| N | Grille | Entités max simultanées | Quota | Loot généré | Blackouts |
|---|---|---|---|---|---|
| 1 | 12 x 12 | 2 | Q(k,1) | 1.6 Q | 1 |
| 2 | 14 x 14 | 3 | Q(k,2) | 1.6 Q | 1 |
| 3 | 17 x 17 | 4 | Q(k,3) | 1.6 Q | 2 |
| 4 | 20 x 20 | 5 | Q(k,4) | 1.6 Q | 2 |

## 7. Extraction
- `AExtractionZone` : 5 s de maintien ininterrompu par joueur, bruit 0.8 (attire), lumière verte visible à 40 m.
- Un joueur extrait devient `Extracted` puis spectateur ; ses crédits portés passent en `TeamBankCredits`.
- Fin de mission réussie si tous les non-`Dead` sont `Extracted`, même avant 480 s.
- Dog Tag d'un mort rapatrié : +50 BR. Cadavre abandonné : -80 BR. Un `Downed` non réanimé en 45 s devient `Dead`.
