# Backlog et prompts exécutables

Ces prompts commandent des travaux futurs ; ils ne signifient pas qu'ils ont été exécutés pendant l'audit. Copier le contrat commun puis **un** prompt dans chaque tâche. Les références Dxx sont détaillées dans [le diagnostic](01_DIAGNOSTIC.md).

## Contrat commun à joindre à chaque tâche

> Travaille sur M.E.G. : Reclamation, Unreal C++, dans le respect de AGENTS.md. Lis docs/audit-2026-09-10/README.md et la section liée à ton ticket. Commence par relever commit, modifications locales et fichiers réservés ; préserve le travail des autres. Vérifie les constats dans l'état actuel avant de corriger. Distingue implémenté, testé et proposé. Ne fais pas de refonte hors périmètre. Tout changement architectural reçoit la revue ue_cpp_architect ; toute nouvelle entité/objet passe par blender_3d_modeller → Blender → FBX → Unreal. Un .uasset/.umap a un seul éditeur. Aucun nettoyage destructif ni fermeture globale d'UnrealEditor. Livre un diff limité, une migration compatible si nécessaire, des preuves liées à chaque critère et les limites restantes. Le Sentinel exécute la suite dans un environnement isolé, avec rapport nominatif ; ne remplace jamais une preuve par un code de sortie ou une déclaration « 100 % ». Si les tests sont absents, bloqués ou interrompus, rapporte-le. Ne commit que tes fichiers explicitement, sans ajouter les changements préexistants.

## Ordre et dépendances

| Ticket | Responsable | Dépendances | Taille relative |
|---|---|---|---|
| P00 | Coordinateur + architecte | Aucune | Petite |
| P01 | ue_cpp_architect | P00 | Moyenne |
| P02 | ue_cpp_architect + bestiary_ai_engineer | P00 | Petite/moyenne |
| P03 | qa_release_sentinel | P00 ; bloque clôture P01/P02 | Moyenne |
| P04 | blender_3d_modeller | P00, règle Hound figée ; P02 pour intégration impact | Grande |
| P05 | bestiary_ai_engineer | P02, P04 | Moyenne |
| P06 | procgen_designer | P00, interface validée P01 | Grande |
| P07 | UI/gameplay + architecte | P00 ; contrats P01 | Grande, par écran |
| P08 | audio_ambience_engineer | P00, P05 pour menace | Moyenne |
| P09 | gameplay + architecte | P01, P03, P06 | Moyenne |
| P10 | réseau + architecte | P01, P09 | Grande |
| P11 | bestiary_ai_engineer + Blender | P02, P05, P06, P08 | Grande, une espèce à la fois |
| P12 | performance + procgen | P04–P09 | Moyenne |
| P13 | QA + game designer | P03–P09, P12 | Plusieurs séances |
| P14 | pipeline/assets + QA | P00 | Moyenne |
| P15 | coordinateur/lore | P00 | Moyenne |
| P16 | release + QA | P10–P15 selon scope | Grande |

Chemin critique : P00 → P01/P02 avec P03 → P04/P05/P06/P07/P08 → P09 → P12/P13 → décision de slice. P10 avant annonce de coop Internet. P11 seulement après preuve d'une rencontre Hound finie. Tailles relatives, sans promesse d'heures ni jours.

P03 valide la fiabilité du runner, même s'il révèle des régressions préexistantes explicitement rapportées. Le nouveau test de distance peut rester rouge jusqu'à P06 : cela ne prouve pas un échec du runner. P01/P02 peuvent recevoir une revue de leurs critères ciblés, sans certification globale ; le passage complet reste obligatoire après correction des défauts, avant slice/release. La production de l'asset P04 peut commencer avant P02, mais son impact intégré ne peut être validé qu'après P02.

## P00 — Une spécification active et des preuves

> Inventorie capacités, cartes techniques/variantes/biomes, entités, outils et leurs preuves. Crée docs/SCOPE_ACTUEL.md avec colonnes prévu/implémenté/chargé/joué/réseau/cook ; conserve les documents historiques. Résous explicitement le conflit Smiler AGENTS/GDD, la signification de nouvelle campagne/continuer, le solo et la cible de slice. Propose comme première cible hub + Level 0 + Hound + lampe/loot/fusible/extraction, puis extension contrôlée. Crée un registre de décisions et relie README au document actif. Ne déclare aucune capacité fonctionnelle sans preuve. Acceptation : chaque élément a un statut, un responsable et une prochaine vérification ; les nombres de cartes et de biomes sont distincts ; aucun changement C++ requis dans ce ticket.

## P01 — Interactions réseau et mutations sensibles

> Corrige D01/D03/D20/D21. Reproduis d'abord porte côté client distant, réanimation hors portée et mutation de poids/sanity sans objet valide. Fais passer l'intention via un acteur possédé ; le serveur valide portée, visibilité, état, coût et propriété. Introduis un descripteur d'interaction commun en migrant porte puis loot puis autres objets, avec compatibilité Blueprint. Corrige l'interpolation porte et la fermeture bornée. Acceptation : hôte + deux clients ouvrent/ferment la même porte ; deux pickups simultanés donnent un seul propriétaire ; une requête rejouée ne double rien ; réanimation distante/soin arbitraire rejetés ; fermeture stable à 30/60/144 FPS et RTT 200 ms. Livre preuves réseau et tests adversariaux, sans implémenter le matchmaking dans ce ticket.

## P02 — Un seul impact de mêlée

> Corrige D02/D19 dans la classe entité et le contrôleur. Formalise autorisation d'attaque selon état de l'espèce, cible valide et fenêtre. L'impact vient de l'AnimNotify serveur avec trace d'occlusion et identifiant d'attaque/victime ; supprime l'application directe redondante. Si un fallback de développement est nécessaire, il produit exactement le même nombre de dégâts et ne s'exécute pas en plus du montage. Acceptation : cible sans obstacle reçoit un impact ; cible derrière mur zéro ; notify répété zéro impact supplémentaire ; cible morte/cachée selon règle/calmée non attaquée ; absence montage traitée explicitement. Documente les signatures Blueprint touchées.

## P03 — Runner QA fiable et sans génération implicite

> Corrige D12/D13/D17/D25/D26. Classe les 30 tests et leurs effets, retire la génération d'assets des SmokeFilter et isole les fixtures. Ajoute rapport structuré nominatif et timeout ; refuse zéro test, tests manquants, erreur et interruption. Corrige le filtre/ordre des commandes seulement après vérification de l'exécution réelle sur le moteur local. Sépare Auto_Check de présence, lancement et gameplay ; remplace « 100% opérationnel » par la portée réelle. Packaging ne ferme plus globalement les éditeurs. Acceptation : un test volontairement échoué fait échouer le runner, un filtre vide aussi ; la suite ne modifie pas Content ; les contrôles 80 m utilisent chemin en cm ; rapport attendu/exécuté/pass/fail/skip archivé. Aucun contournement pour afficher 30/30.

## P04 — Hound : asset jusqu'à l'intégration

> Produis un Hound complet à partir de sa fiche approuvée. Inspecte d'abord le SM existant et les overrides du BP_Hound ; réutilise ce qui fonctionne. Pipeline Blender → FBX → Unreal avec échelle mesurée, normals/UV/PBR, rig et skinning. Livre idle, marche, course, anticipation, impact et récupération, AnimBP, PhysAsset, sockets et notifies. Retire la dépendance au fallback statique uniquement quand le remplacement est branché. Acceptation : skeletal mesh réellement chargé, silhouette lisible à 15 m dans Level 0, pieds et capsule cohérents, un impact synchronisé, capture en déplacement/poursuite/attaque, cook sans référence absente. Pas d'autre créature dans ce ticket.

## P05 — Audition et comportement Hound

> Corrige D18 et implémente la fiche Hound du scope actif. Remplace sélection RMS globale par stimuli localisés avec distance, occlusion, intensité perçue, timestamp et mémoire. Choisis la cible selon stimulus, pas l'ordre de liste des joueurs. États attente, suspicion, recherche, poursuite, attaque, récupération ; rupture de poursuite expliquée. Acceptation : micro/bruit éloigné ne déclenche pas de charge ; porte fermée atténue ; leurre attire à sa position ; perte de visibilité lance recherche bornée ; deux joueurs ne font pas osciller la cible à chaque frame ; règle du regard testée et écrite dans le manuel.

## P06 — Une expédition procédurale fiable

> Travaille uniquement Level 0 et son contrat de mission. Réutilise le générateur ; corrige D23/D24. Sépare graphe de mission, assemblage des salles, puzzles, décor et navigation. Seed/version/hash et flux aléatoires indépendants ; ISM sols/cloisons/néons. Ajoute boucle de détour, raccourci retour, repère et zone d'objectif. Après placement et fermeture de portes, vérifier chemin objectif/extraction, minimum 8 000 cm parcourables et accessibilité loot/clé/fusible. Si vingt essais échouent, utiliser fallback validé ou erreur explicite, jamais dernier layout invalide. Acceptation : campagne 1 000 seeds par configuration avec reproduction des échecs, nav testée avec capsule/portage, hash clients égal, aucun Shift bloquant. Pas de nouveau biome.

Précision d'acceptation pour P06 : les **8 000 cm minimum concernent le chemin praticable du spawn à l'extraction**. Vérifier séparément l'accès à l'objectif ; ne pas appliquer ce seuil seulement au trajet objectif → extraction.

## P07 — Menus, options et prompts lisibles

> Implémente section 4–5 de 02_DESIGN progressivement, un écran par PR. Commence par corriger D06/D07/D08/D09 : nouvelle campagne distincte de continuer, options réellement appliquées, pause solo/multi, prompts selon rebind. Conserve direction terminal ambre et supprime l'effet sur le texte si preset confort. UMG avec FText/String Tables et navigation clavier/manette, back universel, loading/error/cancel. Acceptation : test de chaque bouton ; changement volume et sensibilité mesurable puis conservé au restart ; résolution revert 15 s ; 720p/1080p/1440p/ultrawide et UI150% sans texte coupé ; pseudo-localisation +30% ; fermer terminal rend les contrôles. Ne présente aucun bouton de session comme fonctionnel avant P10.

## P08 — Audio utile, VoIP et accessibilité

> Écoute les sons existants en contexte et garde ceux qui conviennent ; ne condamne pas la synthèse par principe. Réalise un paysage Level 0 et une signature Hound, avec sources/licences. Relie événement sonore à acoustique et stimulus IA ; applique distance/occlusion/réverbération selon portes/pièces. Vérifie proximité et talkie sur deux clients, routing morts/vivants, niveaux et mute. Ajoute sous-titres optionnels des événements effectivement audibles sans localisation parfaite à travers murs. Acceptation : comparatif porte ouverte/fermée, voix intelligible sous alarme, réglage voix réellement efficace, aucune écriture du micro sur disque, micro absent n'empêche pas de jouer. Mimétisme vocal hors périmètre.

## P09 — Objectif, extraction, économie et sauvegarde

> Termine une boucle : équipement de base → mission fusible → loot facultatif → extraction → bilan → progression hub. Serveur seul crédite et consomme ; transactions idempotentes. Campagne hôte et acquis invités explicités. Corrige restauration additive D22 ; save versionnée, atomique, backup et retour d'erreur lisible. Acceptation : après trois échecs une run reste viable ; double déclenchement extraction ne double pas récompense ; save corrompue gérée ; restauration 37 PV/42 sanity exacte ; solo réalise tout sans coéquipier obligatoire. Mesure la durée de run et garde les prix comme paramètres de données.

## P10 — Sessions et reconnexion honnêtes

> Implémente le backend réellement choisi par scope et disponible, sans inventer un SDK. États create/find/join/travel/error ; interfaces UI liées à événements. D22 : identité stable différente du pseudo ; reconnexion rétablit état absolu et propriété inventaire sans duplication. Si migration d'hôte non implémentée, retour propre au menu et message clair. Acceptation : deux machines hors LAN, invitations répétées, salon plein, version incompatible, annulation et perte réseau ; aucune save partagée par homonymes ; join tardif reçoit portes/loot actuels. Aucun secret/token dans commit ou logs. Mesure RTT et erreurs réelles.

## P11 — Smiler, puis Clump seulement si prêt

> Une espèce par tâche. Pour Smiler, résous d'abord contrat AGENTS/GDD via P00 ; distingue adaptation du lore. Corrige D16, perception LOS/portée/cône, signaux d'alerte, contre-jeu et attaque P02. Produis silhouette/animation via Blender et sons via audio. Pour Clump, exiger secours solo et absence de piège sur route unique avant implémentation. Acceptation : joueur peut identifier danger et réaction attendue, aucun déclenchement à travers mur, chaque combinaison lumière/regard vérifiée, aucune mort sans indice préalable dans introduction, captures hôte/client. Évaluer compatibilité des deux règles avant spawn simultané.

## P12 — Mesures et presets

> Capture une baseline packagée sur machine identifiée, scènes et seeds de 03_PRODUCTION. Mesure CPU/GPU frame, p95/p99, RAM/VRAM, génération, shaders et réseau à quatre joueurs. Choisis la correction du plus gros coût démontré, puis compare même scène avant/après. Crée Low/Medium/High cohérents avec indices gameplay. Acceptation : résultats reproductibles, réglages persistants, aucune géométrie/menace essentielle retirée en Low ; cible 1080p60 documentée comme atteinte ou non. N'invente pas des FPS pour une carte graphique non testée et ne désactive pas aveuglément Lumen/RT.

## P13 — Playtest indépendant et décision

> Prépare build et protocole section 12 de 02_DESIGN. Recrute de nouveaux joueurs avec le propriétaire ; aucune session humaine ne peut être simulée et déclarée réelle. Observe première action, compréhension objectif/ennemi, navigation, morts injustes et décisions de retour. Collecte version/seed et retours avec accord, sans voix enregistrée par défaut. Compare aux cibles proposées, classe bugs et problèmes de design séparément. Acceptation : notes anonymisées de sessions réellement tenues, trois corrections prioritaires motivées, décision élargir/itérer avec raisons. Si aucun participant disponible, livre protocole prêt et statut non exécuté.

## P14 — Références, nettoyage et reproductibilité

> Inventorie références Unreal hard/soft, cook et usages scripts avant suppression. Candidats : Weapons template, cartes variantes, plugins et fix scripts ; aucune suppression par nom seul. Utilise Asset Registry/Reference Viewer et cook ; déplacer dans Unreal avec redirectors si nécessaire. Range scripts dans scripts/ existant et mets à jour callers/MCP. Générateurs avec dry-run, portée explicite, hash/version et rapport. Acceptation : zéro référence cassée, import reproductible en copie isolée, aucune modification d'asset artiste ignorée, taille cook avant/après. Pas de migration Git historique ou achat d'assets dans cette tâche.

## P15 — Lore, crédits et contenu public

> Crée registre des sources utilisées pour chaque entité/biome/texte/image/son avec auteur, version/date, URL, licence et adaptation. Utilise 05_SOURCES comme départ et vérifie chaque cas ; ne traite pas Backrooms comme une licence unique. Distingue règles M.E.G. de versions Wikidot/Fandom ; rédige notices et crédits correspondants aux contenus effectivement utilisés. Signale les questions de périmètre de licence à résoudre avant distribution sans affirmer automatiquement que tout le code C++ doit changer de licence. Acceptation : aucune image/son de provenance inconnue annoncé prêt, chaque adaptation documentée, manuel cohérent avec gameplay, liste concrète des points non réglés.

## P16 — Build candidate et préparation publication

> À partir d'un commit propre validé, compile Development Win64 et Shipping Win64 selon AGENTS, exécute QA isolée et packagé autonome rendu ; aucune fermeture globale des éditeurs. Teste installation propre, menus, options, voyage, extraction, retour, save et quitter sans console/debug. Prépare captures/trailer uniquement depuis cette build, fonctionnalités Steam exactement supportées et exigences matérielles mesurées. Acceptation : rapport nominatif, zéro avertissement exigé ou échec déclaré, package checksum/version, liste limites, procédure rollback. Toute publication commerciale externe supplémentaire demande le mandat correspondant ; ne confonds pas push Git du code et sortie publique du jeu.
