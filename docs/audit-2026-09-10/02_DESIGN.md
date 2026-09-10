# Plan de design proposé

Toutes les durées, distances de confort et cibles de test de ce document sont des **valeurs de départ à ajuster en playtest**, sauf la contrainte de 80 m imposée par AGENTS.md. Ce document propose une direction ; il ne décrit pas l'état actuel du jeu.

## 1. Une identité identifiable

Promesse : « Réparer un chemin de retour dans les Backrooms tout en ramenant de quoi maintenir votre base. » La maintenance doit modifier une décision : remettre le courant révèle la sortie, mais fait fonctionner les néons qui exposent l'équipe. Porter un composant précieux prive le joueur de sa lampe. Un détour silencieux consomme du temps.

Conserver l'horreur sérieuse de l'environnement et laisser les joueurs produire leur humour. Éviter d'ajouter des blagues écrites à chaque interaction. Le contraste vient d'une équipe débordée par une situation crédible, pas d'un écran saturé de références internet.

La fiche de [Lethal Company](https://store.steampowered.com/app/1966720/Lethal_Company/) présente récupération, quota, outils et entraide. [Escape the Backrooms](https://store.steampowered.com/app/1943950/Escape_the_Backrooms/) privilégie traversée de niveaux et évitement des dangers. Pour M.E.G., je recommande de concentrer la différence sur la remise en fonctionnement de lieux et les choix de retour ; c'est une proposition, pas une formule commerciale prouvée.

## 2. Périmètre par étape

| Étape | Contenu disponible | Condition pour agrandir |
|---|---|---|
| Prototype intégré | Hub, Level 0 compact, Hound, lampe, loot, fusible, extraction | Boucle jouable hôte + client, aucun blocage critique |
| Vertical slice | Même biome fini ; Hound puis Smiler ; Clump seulement après secours solo ; cinq fonctions d'équipement | Silhouettes, sons, UI, performance et retours de nouveaux joueurs validés |
| Alpha | Level 1 puis Level 37, missions variées, progression courte, reconnexion contrôlée | Chaque biome ajoute une décision distincte et réutilise le pipeline |
| Extension | Autres biomes et entités au cas par cas | La nouvelle mécanique justifie son coût ; aucune dépendance bloquante à du contenu futur |

Les cinq fonctions de la slice : lampe multispectre avec batterie, talkie, consommable d'eau d'amande, leurre sonore, scanner simple. Le fusible est un objet de mission, le loot appartient à une catégorie séparée. Le LIDAR avancé, le strobe autonome et l'ancre arrivent lorsque leurs situations de choix existent. Aucun inventaire Tetris requis : quatre emplacements lisibles et un portage lourd suffisent pour mesurer la boucle.

## 3. Parcours des dix premières minutes

| Moment indicatif | Ce que vit le joueur | Information transmise |
|---|---|---|
| 0:00–0:45 | Réglages essentiels, rejoindre/créer | Langue, confort visuel, sortie audio, micro facultatif |
| 0:45–2:00 | Petit hub, matériel de base, porte d'essai | Regarder, interagir, porter, déposer, parler/pinger |
| 2:00–3:00 | Choix d'une mission clairement affichée | Objectif principal, condition de retour, risque et équipement fourni |
| 3:00–5:00 | Sas puis zone calme et premier loot | Retrouver le sas et lire un repère sans flèche omnisciente |
| 5:00–7:00 | Panne simple et premier indice sonore | Remettre le courant produit un changement visible et audible |
| 7:00–10:00 | Menace annoncée, décision de contourner/laisser du loot | Comprendre une règle de survie et pouvoir revenir |

Ne pas faire dépendre ce tutoriel d'une génération totalement libre. Utiliser un contrat de mission introductive et des salles balisées, puis ouvrir la variation. Aucune mort obligatoire pour apprendre une règle. Le tutoriel peut être rejoué et ignoré par un habitué.

## 4. Menu et direction UI

Conserver le terminal ambre, mais faire passer le texte devant les effets. Fond sombre chaud, titres ambre, texte courant ivoire, danger rouge avec pictogramme ; éviter les cinq couleurs de statut concurrentes. Police proportionnelle lisible pour explications, monospace pour valeurs et identifiants. Texture CRT sous les éléments interactifs, pas de scintillement sur les lettres.

Disposition cible 16:9 : zone de navigation à gauche, panneau de contexte au centre/droite, état connexion et version discrète en bas. Fond 3D facultatif avec qualité indépendante. Réglages accessibles avant toute connexion. Taille UI 100/125/150 %, zones cliquables d'au moins 44 pixels à l'échelle de référence 1080p, focus toujours visible. Valider 1280×720, 1920×1080, 2560×1440, ultrawide et texte agrandi.

| Écran | Contenu et actions | États indispensables |
|---|---|---|
| Accueil | Continuer campagne, Jouer, Paramètres, Crédits, Quitter | Continuer indisponible sans save, raison affichée |
| Jouer | Héberger, Rejoindre, Entraînement solo | Connexion, hors ligne, version incompatible, annuler |
| Héberger | Nom, visibilité privée/amis/publique, difficulté, sauvegarde hôte | Création en cours, échec explicite, réessayer sans doublon |
| Rejoindre | Liste de salons ou invitation ; connexion IP dans section avancée | Recherche vide, plein, expiré, mauvais mot de passe, timeout |
| Salon/hub | Quatre places, état prêt, test vocal, mission, inventaire commun | Départ refusé avec raison ; nouveau joueur ; départ de l'hôte |
| Pause | Reprendre, options, liste joueurs/mute, retour menu | Solo réellement en pause ; multi avertit que le monde continue |
| Mort | Cause compréhensible, caméra d'allié, retour au hub après run | Joueur sauvé entre-temps, dernier survivant, fin de mission |
| Débrief | Objectif, valeur ramenée/perdue, dépenses, prochaine action | Crédit transactionnel unique, tout le monde voit le même résultat |

Une nouvelle campagne demande une confirmation seulement si elle efface une progression. Quitter une session en avertit les conséquences ; l'UI ne prétend pas effectuer une migration d'hôte absente. Retour arrière universel clavier/manette ; pas de capture clavier résiduelle après fermeture du terminal.

### Réglages avec effet observable

- Vidéo : résolution, fenêtré sans bordure, qualité, limite FPS, VSync, luminosité calibrée ; retour automatique à l'ancien mode après 15 secondes sans confirmation.
- Confort : FOV, motion blur, grain, aberration, secousses, head bob, flashes. Preset sobre disponible dès le premier écran.
- Audio : master, ambiance, effets, voix ; choix entrée/sortie si backend le permet ; test micro local, push-to-talk, seuil et indicateur de transmission ; conserver le choix après redémarrage.
- Contrôles : rebind AZERTY/QWERTY/manette, sensibilité X/Y, inversion, toggle/hold sprint et accroupissement, résolution des conflits. Les invites utilisent les touches réellement assignées.
- Accessibilité : sous-titres d'événements, taille/contraste, statuts par forme et texte, alternative aux pressions répétées, réglages sans micro. Adapter la précision des indices visuels à l'audibilité réelle, sans révéler une entité inaudible derrière plusieurs cloisons.

Ces besoins prolongent les [Game Accessibility Guidelines](https://gameaccessibilityguidelines.com/basic/) ; les dimensions et presets ci-dessus sont spécifiques à ce plan.

## 5. Interactibles : un contrat commun

Chaque objet expose : nom localisable, verbe, touche effective, disponibilité, motif d'échec, durée, coût, état courant et éventuelle action secondaire. Le HUD lit un descripteur ; le serveur décide du résultat. Visée stable avec petite hystérésis pour éviter que le texte saute entre deux objets. Portée initiale 200 cm à régler par catégorie ; distance et ligne de vue recalculées au serveur.

| Objet | Geste | Retours et règles |
|---|---|---|
| Porte | Appuyer ouvrir/fermer ; secondaire ouverture prudente | Poignée/charnière, son selon effort ; obstacle et verrou expliqués ; deux joueurs ne provoquent pas un va-et-vient infini |
| Loot | Appuyer prendre, déposer sans lancer, lancer distinct | Nom/poids/valeur estimée ; confirmation main/inventaire ; un seul propriétaire serveur |
| Objet lourd | Maintenir saisir ; déposer rapide prioritaire | Main/outil indisponible expliqué ; encombrement sans physique instable ; voie solo de transport |
| Fusible | Insérer compatible ; retirer si autorisé | Emplacement visible, courant propagé, feedback lumineux ; impossible de consommer deux fois |
| Valve | Maintenir tourner, relâcher interrompre | Mouvement, jauge mécanique, vapeur réellement modifiée ; pas de QTE répétitif |
| Terminal | Appuyer utiliser, retour fermer | Focus UI explicite ; achat affiche coût et solde ; double clic ne double pas l'achat |
| Cachette | Entrer/sortir | Occupation serveur ; visibilité vers extérieur limitée ; secours si transition interrompue |
| Allié au sol | Maintenir secourir | Progression interruptible et coût annoncé ; pas de soin distant |
| Extraction | Déposer requis, activer puis attendre | Liste conditions remplies/manquantes et compte à rebours partagé |

Supprimer les actions génériques « Interagir » lorsque le verbe peut être précis. Éviter le contour fluorescent permanent : forme, contraste local et petit highlight de visée. Un échec donne une raison courte (« mains occupées », « fusible requis »), pas seulement un bip.

## 6. Boucle, économie et solo

Durée initiale d'une run : 12–18 minutes, préparation 1–3 minutes. Une mission principale répare/alimente le retour ; le loot facultatif finance la base. Placer un raccourci déverrouillable entre aller et retour. Le joueur choisit quand repartir après l'objectif, avec risque croissant signalé ; le premier essai ne doit pas finir brutalement par un timer invisible.

Économie initiale à prototyper : 100 unités de récompense de mission, loot variable, équipement de survie minimal gratuit après échec. Les prix se règlent sur le gain net médian mesuré, pas sur l'ancien chiffre arbitraire de 2 000 crédits. Ne pas cumuler dette punitive, équipement indispensable payant et wipe complet. Trois échecs successifs doivent encore permettre une run viable.

Chaque amélioration débloque un choix : deuxième poste de recharge, stockage, analyse de données, route supplémentaire. Différer les bonus permanents de vitesse/survie qui creusent l'écart entre amis. La progression de campagne appartient à l'hôte ; définir explicitement ce que les invités conservent avant toute sauvegarde réseau.

Solo : objectif ajusté, transport lourd plus lent mais possible, alternative de libération au Clump, puzzle sans deux interrupteurs simultanés obligatoires. Duo/trio/quatuor modifient densité et besoins de transport, pas automatiquement les PV des monstres. Un joueur mort conserve une caméra d'observation limitée ; pas de radar révélant toutes les menaces aux vivants.

Sanité : une échelle lisible, trois paliers avec symptômes graduels ; éviter vision brouillée + input dégradé + fausses portes obligatoires simultanés. Les hallucinations restent cosmétiques/client-locales et ne remplacent jamais une collision ou sortie indispensable. Eau d'amande : choix entre soin, vente et secours, avec effets identiques au serveur.

## 7. Entités : règles lisibles avant quantité

Chaque fiche doit contenir perception → suspicion → recherche → poursuite → attaque annoncée → récupération, moyens de rupture, contraintes de spawn, comportement solo, animation, son et tests. Une silhouette reconnaissable doit correspondre à une règle stable. Une attaque n'applique de dégâts qu'au moment de l'impact validé par AnimNotify, une fois par cible ; un obstacle opaque bloque l'attaque.

| Entité / ordre | Silhouette et indice | Règle proposée et contre-jeu | Garde de design et test |
|---|---|---|---|
| Hound / première | Quadrupède humanoïde bas, griffes et souffle irrégulier | Entend stimulus localisé, inspecte dernière position, poursuit après confirmation ; casser LOS, marcher, utiliser leurre | Regard intimidant temporaire seulement si adopté dans contrat ; ne pas copier une immobilisation infinie. Silence derrière obstacle : recherche puis abandon. |
| Smiler / deuxième | Yeux et sourire, masse corporelle presque invisible, distorsion identifiable | Conformément à AGENTS actuel : illumination OU regard prolongé provoque avertissement puis charge ; détourner regard, éteindre, rompre LOS | **Adaptation gameplay** à distinguer du wiki et du GDD actuel. Fenêtre indicative 0,8–1,2 s évite mort immédiate à la découverte. Jamais de perception à travers mur. |
| Clump / troisième conditionnelle | Membres au bord d'une cavité, frottement et trace | Embuscade fixe, zone d'attaque annoncée, libération alliée ou ressource/effort solo | Pas de spawn sur passage unique obligatoire ; aucun softlock solo ; impossibilité de saisir à travers porte. |
| Watcher / plus tard | Silhouette verticale immobile, anomalie de cadrage | Exposition visuelle prolongée fait monter une jauge ; détourner regard la fait redescendre | Éviter combinaison avec Hound exigeant regard dans le même cône sans échappatoire. |
| Duller / plus tard | Déformation et empreintes, contour scanner | Scanner révèle trajectoire et passages ; indice non instrumental permet de fuir | Ne pas exiger un outil indisponible pour finir la mission. |
| Skinwalker / différé | Imitation proche du joueur avec deux indices fiables | Appels trompeurs, comportement spatial imparfait ; vérification par défi collectif | Première version utilise voix synthétiques enregistrées pour le jeu. Mimétisme des joueurs seulement opt-in, RAM bornée, purge, aucun upload automatique. |
| Wretch / différé | Humanoïde cassé, déplacement irrégulier | Menace lente de proximité, attire l'équipe vers route alternative | Supprimer/fusionner si aucun choix distinct du Hound ; pas de remplissage automatique. |
| Deathmoth / différé | Ailes autour d'une source, battement reconnaissable | Défend une zone éclairée ; route sombre ou extinction | Variante plus tard ; différencier clairement sa règle de celle du Smiler. |
| Jerry / différé | Oiseau et zone sonore repérable | Hypnose graduelle, rupture d'exposition et secours | Pas de longue confiscation des commandes sans alternative solo. |
| Partygoer / différé | Ballon et silhouette propre, motif sonore rare | Contamination visible avec fenêtre de traitement et choix de séparation | Reporter conversion PvP ; aucun contrôle caché d'un joueur sans explication. |
| Hydrolitis / avec Poolrooms | Remous directionnels et mouvement sous la surface | Suit perturbations de l'eau ; rejoindre une berge ou ralentir | Pas de cumul eau lente + saisie + route unique ; test lisibilité en qualité basse. |

Le tableau couvre les noms rencontrés dans code/documents, **pas une promesse de onze créatures à produire**. Pour Hound et Smiler, les références Wikidot sont documentées dans [les sources](05_SOURCES.md). Les autres règles sont des propositions M.E.G., sans revendication de fidélité canonique.

Tests d'une espèce : cible visible/cachée/morte, mur fermé/ouvert, deux joueurs en stimuli opposés, lampe UV/standard/éteinte, perte de chemin, entrée tardive client, baisse FPS, interruption animation et recharge de niveau. Reconnaissance de silhouette à 15 m à tester avec des joueurs, pas à déclarer depuis un render Blender.

## 8. Rythme et directeur de tension

Séparer population, comportements et rythme. Le directeur choisit une fenêtre et un budget de menaces, sans modifier secrètement leurs règles. Phases proposées : calme → indice → pression → incident → récupération. Mesures : proximité menace, poursuites actives, blessures et séparation. Après une poursuite, réserver une respiration ; une équipe blessée ne reçoit pas automatiquement une nouvelle horde.

Le travail de [Valve sur le directeur de Left 4 Dead](https://cdn.akamai.steamstatic.com/apps/valve/2009/ai_systems_of_l4d_mike_booth.pdf) fournit une référence pour l'alternance d'intensité. Ici, proposer au départ une seule menace active dans l'introduction et deux dans la slice. Le directeur n'a pas le droit d'apparaître dans la vue d'un joueur, de bloquer toute sortie ou d'annuler un contre-jeu réussi.

## 9. Cartes procédurales à trois échelles

**Macro :** un graphe intentionnel relie insertion, repère, objectif, zone risquée, raccourci et extraction. Une boucle utile, deux choix de route, branches facultatives récompensées. La distance minimale de 80 m est une longueur de chemin praticable après placement, pas une distance à vol d'oiseau. Éviter la taille comme substitut à la densité d'intérêt.

**Méso :** salles composées à la main, connecteurs normalisés, seuils de visibilité, endroits où se croiser et déposer le loot, niches d'évitement. Définir un kit de 12 salles/modules pour commencer : couloir droit, angle, T, croisement, petite salle, salle longue, pilier, niche, maintenance, objectif, sas et landmark. Trois variantes décoratives ne comptent pas pour trois salles de gameplay.

**Micro :** papier peint, raccords, traces, humidité, indices sonores, accessibilité des poignées, collisions des props. Les surfaces de passage sont dégagées ; un détail peut raconter mais ne doit pas coincer le portage. Répéter les motifs d'architecture, varier les accidents et repères.

Pipeline recommandé : seed et version → graphe de mission → placement de salles → connexions → validation chemin → puzzles et objets → décor/ISM → navigation → validation finale → admission des joueurs. BSP et assemblage contraint suffisent si le résultat est bon ; WFC n'est pas une obligation de refonte.

Seed, version du générateur et hash du layout enregistrés au départ. Flux aléatoires séparés pour géométrie, loot, menaces et décor ; changer une texture ne doit pas déplacer l'extraction. Aucun client ne décide sa propre version de porte logique. Late join reçoit l'état courant, pas seulement la seed initiale.

Liminal Shift : commencer par des portes/routes logiques. Aucun changement sous un joueur, loot critique ou ligne de vue de l'équipe ; revalider objectif et sortie avant commit serveur. Si la mutation échoue, l'annuler entièrement. Garder un chemin sûr même pendant collapse ; perte d'une route annoncée par un indice local.

## 10. Direction des onze biomes

| Biome / ordre | Matières, lumière, son | Différence mécanique | À éviter |
|---|---|---|---|
| 0 / slice | Moquette humide, papier jauni, néons ; ronflement changeant par secteur | Lire répétition et repères, remettre alimentation | Couloir uniforme interminable et jaune saturé partout |
| 1 / alpha | Béton, cages, métal froid, brouillard modéré | Transport et raccourcis de service | Copier Level 0 avec une texture grise |
| 37 / alpha | Carrelage clair, eau lisible, réverbération longue | Vitesse/bruit dans l'eau, berges sûres | Beau rendu sans décision ; opacité eau masquant tout |
| 2 / extension | Tuyaux, vapeur, lumière de maintenance | Valves et cycles de pression | Vapeur létale sans signe précurseur |
| 3 / extension | Câblage, armoires, arcs rares | Gestion de circuits et zones alimentées | Même puzzle fusible répété sans variante |
| 4 / extension | Bureaux vides, fenêtres diffuses, faux silence | Orientation par signalétique et archives | Dégâts de sanité inévitables sans contre-jeu |
| 6 / extension | Noir, rares réflexions, acoustique de volume | Énergie/scanner et chemin acoustique | Écran réellement illisible ou outil obligatoire non fourni |
| 8 / extension | Roche, humidité, fractures | Scanner, verticalité courte, goulets contournables | Collision/franchissement imprévisibles |
| 9 / extension | Pavillons sans vie, éclairage parcellaire | Détours extérieurs et fenêtres surveillées | Carte immense vide et trop coûteuse à rendre |
| 10 / extension | Blé, vent, points hauts | Ligne de vue perdue et rendez-vous de groupe | Visibilité variant injustement selon qualité graphique |
| ! / final/bonus | Couloir rouge, alarmes, portes latérales | Course courte à décisions de trajectoire | Course incompatible avec poids/stamina sans compensation |

Ces interprétations sont des briefs originaux de gameplay. Les noms des variantes de map comme `Lvl_99_RunForYourLife` doivent être reliés à un identifiant de biome explicite plutôt que compter un biome supplémentaire.

## 11. Art, animation et son

Style conseillé : environnement crédible mais simplifié, contrastes maîtrisés, équipements M.E.G. usés et lisibles, créatures à silhouettes fortes. Choisir une référence d'échelle humaine et une densité de textures cohérente. Éviter d'empiler VHS, brouillard, grain et motion blur pour cacher les faiblesses.

Animation minimale par entité mobile : idle, déplacement lent/rapide, orientation, alerte, anticipation, impact, récupération. Les pas suivent les contacts ; les mains du joueur montrent prise/dépose/utilisation ; les portes ont son et mouvement concordants. Priorité au timing et à la silhouette avant détails de surface. Pas de mort animée obligatoire pour un monstre invulnérable si le design ne prévoit pas sa mort.

Son en couches : fond du biome, sources localisées, événement lointain rare, information de menace. Séparer acoustique ambiante et stimulus IA tout en les faisant partir d'un événement commun. Atténuation et occlusion déterminent ce qu'entend chaque joueur ; les ennemis n'écoutent pas un RMS global de micro sans distance.

VoIP : proximité, porte ouverte/fermée, radio mono filtrée, priorité de lisibilité sous alarme, option de compression dynamique. Tester écouteur gauche/droite, sortie haut-parleur, micro absent, saturation et coupure réseau. Les filtres talkie ne doivent pas rendre toute parole inintelligible. Définir qui entend les morts et l'expliquer.

Écouter les sons procéduraux existants avant remplacement. Conserver ceux qui servent le lieu ; compléter les créatures avec textures organiques si nécessaire. Registre de source/licence pour toute banque. Une boucle de néon excellente est plus utile que vingt cris interchangeables.

## 12. Playtests qui décident réellement

Premier cycle : cinq personnes ne connaissant pas le projet, puis trois groupes coop si disponibles ; petit échantillon qualitatif, pas statistique commerciale. Observer sans expliquer, noter temps jusqu'à première action, hésitations, causes de mort comprises et envie de relancer. Enregistrer seulement avec accord ; ne pas enregistrer le micro par défaut.

Cibles initiales : 4/5 trouvent comment démarrer en moins de deux minutes ; 4/5 expliquent la règle du premier ennemi après rencontre ; aucune run bloquée par objet inaccessible ; majorité des groupes expriment un choix concret de risque/retour. Si l'objectif est raté, corriger l'écran, l'indice ou la carte concernée avant de créer un biome.

Questions de débrief : « Quand savais-tu quoi faire ? », « Qu'est-ce qui t'a tué ? », « Quand as-tu hésité à continuer ? », « Quel objet as-tu choisi de laisser ? », « Qu'aimerais-tu retenter ? ». Éviter « As-tu aimé mon jeu ? » comme seule mesure. Séparer peur voulue, confusion involontaire, difficulté et bug.
