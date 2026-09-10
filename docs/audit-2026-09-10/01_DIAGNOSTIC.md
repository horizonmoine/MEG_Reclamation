# Diagnostic vérifié

## Méthode et limites

Lecture du C++, configurations, scripts et inventaire de Content ; comparaison des trois documents joints ; consultation de sources publiques primaires ; revue architecte et Sentinel. Les numéros de ligne décrivent l'état local du 10 septembre et pourront changer. Aucun Blueprint binaire n'a été inspecté dans l'éditeur ; une affectation Blueprint peut donc compléter ou modifier un défaut C++. Les chemins sont relatifs à la racine du dépôt.

**Vérifié** signifie observable dans les fichiers. **Risque** signifie conséquence probable à reproduire. **Proposition** désigne une décision de design. Aucun de ces termes ne signifie « testé en multijoueur ».

## Constats prioritaires

| ID | Priorité | Preuve locale | Conséquence et prochaine vérification |
|---|---|---|---|
| D01 | P0 | `Source/MEG_Reclamation/Player/ScavengerCharacter.cpp`, `Interact()` vers 1923/1983 appelle directement la porte ; `Objects/LiminalDoorActor.cpp:116`, garde d'autorité | Le chemin C++ client appelle une action qui exige le serveur. Reproduire avec client distant, puis router la requête par un acteur possédé et revalider côté serveur. |
| D02 | P0 | `AI/LiminalEntity.cpp:237-268`, fallback mêlée lance trace et `ApplyDamage` direct | Double application possible lorsque la trace touche ; cible directe pouvant contourner l'occlusion. Une attaque doit produire au plus un impact par victime. |
| D03 | P0 | `Player/ScavengerCharacter.cpp`, `ServerRevivePlayer_Implementation` vers 1229, `ServerRestoreSanity` vers 1135 et `ServerRemoveInventoryWeight` vers 1110 | Réanimation et changements de ressources ne doivent pas croire une intention/quantité fournie par client. Tester distance, état, coût consommé, ownership et répétition. |
| D04 | P1 | `AI/LiminalEntity.cpp:45-46,93-142` ; `Content/Meshes/Hound/SM_Hound.uasset` présent ; pas de `Content/Characters/Bestiary/Hound/SK_Hound.uasset` dans l'inventaire | Le squelette par défaut du Hound ne résout pas à ce chemin. Fallback statique prévu. Vérifier les classes réellement instanciées et leurs overrides avant d'affirmer que tous les monstres sont identiques. |
| D05 | P1 | `Content/Characters/Bestiary/` contient `Hydrolitis/SM_Hydrolitis.uasset` ; BT/BB Hound, Smiler, Clump présents dans `Content/AI/` | Des comportements/assets existent, mais la présence de BT ne prouve ni rig, ni animation, ni intégration. Construire une matrice chargement → apparition → locomotion → attaque → son. |
| D06 | P1 | `UI/LiminalMainMenuHUD.cpp`, `OnNewExpedition()` et `OnContinueGame()` vers 808-835 appellent tous deux chargement puis retour hub | Deux libellés distincts mènent au même comportement C++. Définir partie, campagne et sauvegarde ; éviter un futur reset implicite. |
| D07 | P1 | `UI/LiminalMainMenuHUD.cpp:934-940,966-972` écrit/lit des réglages audio et souris ; recherche de ces clés dans les `.cpp` ne trouve que ce fichier | Persistance observée, application effective non démontrée. Tester variation sonore et rotation caméra, puis relier au runtime si absent. |
| D08 | P1 | `UI/LiminalMainMenuHUD.cpp`, DrawHUD/DrawText, chaînes FString ; pas de LOCTEXT/NSLOCTEXT/StringTable trouvés dans UI | Localisation et layout reposent sur du texte codé en dur. Migrer écran par écran et tester texte long, manette, résolution et taille UI. Canvas n'est pas intrinsèquement incapable d'accessibilité. |
| D09 | P1 | `UI/LiminalPauseMenuComponent.cpp:TogglePause`, appelle `PC->SetPause` sans distinction explicite solo/multi | Reproduire comportement hôte et client ; afficher « la partie continue » en réseau et garder une vraie pause solo. |
| D10 | P1 | `UI/LiminalMainMenuHUD.cpp:OnConnectToHost`, `ClientTravel(JoinIPAddress, TRAVEL_Absolute)` | Connexion IP observée ; découverte de salons Steam et invitations non établies par cette implémentation. Tester hors LAN avant toute promesse de matchmaking. |
| D11 | P1 | `Config/DefaultEngine.ini`, DX12/SM6, Lumen, Virtual Shadows, `r.RayTracing=True`, cible Maximum ; pas de DefaultScalability.ini | Besoin d'une cible matérielle et de captures de profilage. Le drapeau RayTracing ne démontre pas seul que Lumen utilise le ray tracing matériel ; aucune prédiction de FPS possible. |
| D12 | P1 | `Run_Automation_Tests.ps1`, validation par code processus ; 30 déclarations `IMPLEMENT_SIMPLE_AUTOMATION_TEST` trouvées dans `Tests/MegReclamationTests.cpp` | Compter tests découverts/exécutés/réussis dans un rapport. Sortie 0 et existence de 30 déclarations ne prouvent pas 30 exécutions. |
| D13 | P1 | `Tests/MegReclamationTests.cpp:520-521`, AssetPipelineGeneration invoque BuildFullGame/BuildHoundAssets | La suite n'est pas entièrement en lecture seule : elle peut produire des assets. Lancer dans copie isolée ou séparer génération et vérification. |
| D14 | P2 | `Run_Auto_Check.ps1` contrôle 18 noms de maps ; GDD sections 1.8 et terminal annoncent 7 et 11 biomes | Séparer nombre de biomes, variantes et cartes techniques ; un nom de map ne prouve pas le contenu du biome. |
| D15 | P2 | `Content/Weapons/Pistol`, `Rifle`, `GrenadeLauncher` existent | Résidu candidat au nettoyage. Leur présence ne prouve pas que le joueur dispose d'armes. Mesurer inclusion cook et dépendances avant suppression. |
| D16 | P1 | `AI/LiminalEntity_Smiler.cpp` logique de regard/lumière ; AGENTS exige charge si éclairé ou fixé, GDD prévoit immobilisation dans le noir | Contrat contradictoire. Résoudre la règle avant réglage IA et tutoriel ; vérifier également occlusion de la perception. |
| D17 | P2 | `orchestrate_meg_team.py` gère l'absence de SDK avec classes simulées | Un démarrage réussi ne prouve pas que des agents réels ont travaillé. Le rapport doit distinguer simulation et exécution effective. |

Les préfixes abrégés AI/, UI/, Player/, Tests/ et Objects/ dans ce tableau sont sous `Source/MEG_Reclamation/`.

## Compléments de la revue architecte et QA

| ID | Priorité | Preuve sous Source/MEG_Reclamation sauf indication | Action |
|---|---|---|---|
| D18 | P1 | `AI/LiminalEntity_Hound.cpp:58-74` choisit un joueur au-dessus du seuil RMS sans distance/occlusion dans ce chemin | Stimulus acoustique localisé, intensité perçue, expiration et sélection déterministe ; ce fichier est nouveau et non suivi au début de l'audit. |
| D19 | P1 | `AI/LiminalAIController.cpp:85-103` attaque les joueurs proches indépendamment de la perception ; `AI/LiminalEntity.cpp:232` ne filtre pas l'état calmé | Donner l'autorisation d'attaque à la logique d'espèce, avec garde commune au point d'impact. |
| D20 | P1 | `Objects/LiminalDoorActor.cpp:86-94,306` ; `TargetAngle` non répliqué dans `.h:167` | Répliquer transition cohérente ; vérifier absence d'oscillation client. |
| D21 | P1 | `Objects/LiminalDoorActor.cpp:318-324`, pas fixe de fermeture et tolérance sans clamp de dépassement | Tester fermeture à plusieurs FPS ; interpolation bornée. |
| D22 | P1 | `Network/LiminalSessionManager.cpp:28-31,108` identité par nom ; restauration via soin additif (`Player/ScavengerCharacter.cpp:624-625`) | Identité stable, restauration absolue versionnée et transactionnelle. |
| D23 | P1 | `ProcGen/LiminalLevelGenerator.cpp:370-396` conserve dernier résultat après vingt tentatives | Refuser un layout qui ne satisfait pas les 80 m ; fallback validé. |
| D24 | P2 | `ProcGen/LiminalLevelGenerator.cpp:647`, `FMath::FRand` pour scintillement | Décider si cosmétique locale ou reproductible ; séparer flux aléatoires. |
| D25 | P1 | `Package_Shipping_Build.ps1:15` ferme globalement UnrealEditor/LiveCoding | Ne plus fermer les sessions de travail sans sauvegarde ; verrou ciblé et échec explicite. |
| D26 | P1 | `Tests/MegReclamationTests.cpp:744-749`, `DistSq > 4` en cellules | Ne démontre pas 80 m parcourables. Tester distance de chemin en centimètres après placement et changements de portes. |

`TriggerHostMigration` ne diffuse qu'un événement (`Network/LiminalSessionManager.cpp:145-147`). Le branchement Blueprint de la migration, de la stasis et des contrôles mouvement n'est pas vérifié. Ne pas annoncer ces capacités comme opérationnelles.

## Ce qui est réellement encourageant

Le GDD relie la préparation au hub, le poids, la lumière et le retour d'expédition. Cette cohérence donne un critère pour refuser les gadgets inutiles. Les types C++, interfaces, composants, tests de logique et générateurs peuvent servir à terminer cette boucle. Il n'est pas nécessaire de reconstruire tout le projet, de changer de moteur ou de remplacer immédiatement les Behavior Trees.

L'existence de flux d'autorité et de réplication est utile, mais l'architecte a trouvé des chemins incohérents. La formulation « architecture réseau propre » des anciens audits était donc trop globale. Vérifier action par action.

## Corrections importantes aux documents joints

| Affirmation antérieure | Évaluation révisée |
|---|---|
| « 10-15 % de contenu réel », « 5 % à finir », estimation studio de 15 personnes | Pas de mesure ni décomposition justifiant ces nombres. Remplacer par états de maturité et tâches avec preuves. |
| « Tous les monstres ressemblent au Hound » | Fallback commun et chemins manquants établis ; rendu effectif de toutes les classes non observé. |
| « Tous les tests ne sont que des calculs » | Faux comme description exhaustive : un test invoque des générateurs d'assets. Classer réellement chaque test et ses effets de bord. |
| « Le code des sons prouve qu'ils ne font pas peur » | La synthèse se reconnaît dans le code ; sa qualité exige écoute en contexte. Un son synthétique peut être excellent. |
| « Déplacer les scripts est mécanique et sans risque » | Les chemins absolus, imports, MCP et scripts de lancement peuvent casser. Déplacement par lots avec vérification. Réutiliser `scripts/` existant, éviter `Scripts/` concurrent sous Windows. |
| « Plugin activé = inutilisé » | Aucune équivalence. Inspecter usages C++, assets, outils éditeur et dépendances avant retrait. |
| « Pas d'audit trouvé = protocole contourné » | Absence de preuve retrouvée ne démontre pas qui a fait quoi. Dire « certification non étayée par les artefacts examinés ». |
| « Scope court et style simple expliquent le succès commercial » | Hypothèse de design, pas causalité établie. Les références aident à décider, elles ne garantissent pas les ventes. |
| « 5 000–10 000 wishlists déclenchent l'algorithme » | Aucun seuil garanti retenu ici. Suivre la documentation Steam et ses propres conversions. |
| « Lore canonique unique » | Les communautés et versions divergent. Choisir des sources identifiées et publier les adaptations de gameplay. |

## Ce qui manque pour évaluer le jeu comme joueur

Une capture continue d'une run solo et d'une run à quatre, avec son ; une observation indépendante de première prise en main ; un rapport assets chargés/cookés ; des mesures de temps de frame ; un essai connexion Internet et de reconnexion. Jusqu'à ces preuves, la peur, la navigation réelle, la fluidité et la fidélité des animations restent des questions ouvertes.
