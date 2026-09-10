# Plan technique et production

## Architecture : corriger puis découpler

La revue `ue_cpp_architect` recommande des corrections ciblées avant une refonte générale. Conserver le serveur autoritaire, les conventions Epic et les composants existants. Ne pas ajouter GAS, Mass, StateTree ou un framework de réplication pour leur seule disponibilité.

| Domaine | Contrat cible | Migration et preuve |
|---|---|---|
| Interaction | Intention via personnage/controller possédé ; validation serveur ; descripteur commun UI | Une porte d'abord, puis loot et autres objets ; conserver compatibilité Blueprint et tester chaque migration |
| Inventaire | Serveur propriétaire de l'objet, du poids et des transactions | Requêtes répétées/concurrentes ne créent ni loot ni crédits ; un dépôt confirme la libération de propriété |
| Mêlée | Identifiant d'attaque, fenêtre AnimNotify, trace d'impact, victime unique | Trace bloquée par mur ; répétition de notify sans double dégât ; fallback réservé et équivalent |
| IA | Perception et mémoire distinctes de décision et impact | Contrôleur commun ne contourne pas états calmé/caché/inactif ; réglages par espèce dans données |
| Procgen | Layout déterministe versionné et validation finale | Comparer hash entre serveur/clients ; portes et loot dynamiques répliqués |
| Sauvegarde | Schéma versionné, écriture atomique, backup, restauration absolue | Save corrompue, interruption, version ancienne, pseudo dupliqué, nouveau pawn |
| Sessions | Machine d'état création/recherche/join/travel/erreur/déconnexion | Annulation, version incompatible, double invitation, timeout et retour au menu |
| UI | Vue + modèle d'état ; réglages runtime explicites | Migration UMG progressive, focus/manette/localisation vérifiés ; pas de réécriture globale en une tâche |
| Audio | Événement sonore localisé et routes proximité/radio | Profil portes et distances ; pas de capture vocale persistante implicite |

Extraire progressivement du personnage les responsabilités interaction, inventaire et survie après stabilisation. Une extraction par PR, sans changement comportemental simultané. Des références UPROPERTY propriétaires utilisent TObjectPtr ; références non propriétaires TWeakObjectPtr ; validation des pointeurs et des cycles de vie lors de travel/destroy.

## Réseau et données

Matrice minimale : solo ; listen server + 1 client ; hôte + 3 clients ; arrivée tardive si supportée ; perte de connexion ; RTT 0/100/200 ms et perte 0/1/3 % en émulation. Ces valeurs sont des cas d'essai proposés, pas des tolérances déjà atteintes. Vérifier en processus séparés et sur deux machines avant de valider Internet.

Le serveur recalcule portée, LOS, inventaire, coût, état vivant/downed et disponibilité. Le client n'envoie pas « ajoute 100 sanity » mais « utiliser l'objet X ». Identifiants de transaction et débit borné pour achats, soin, extraction et loot. Une validation `_Validate` seule ne remplace pas les règles métier.

Reconnexion : décider d'abord des limites (au hub seulement pour première version, ou réservation temporaire de place). Identité authentifiée quand la plateforme existe, identifiant de session robuste autrement ; jamais le pseudo comme clé unique. Restaurer les valeurs exactes et empêcher duplication d'inventaire. Si l'hôte part sans migration réelle, terminer proprement et préserver la dernière transaction sûre. Ne pas promettre de migration transparente.

## Pipeline d'assets

Chaque asset reçoit un identifiant, un propriétaire, une source/licence, un fichier éditable, des paramètres d'export et une destination Unreal. Stati distincts : concept, source prête, importé, branché, rendu vérifié, testé réseau, cook validé. Le statut « terminé » exige toutes les étapes pertinentes.

Pipeline imposé aux nouvelles entités/objets : `blender_3d_modeller` → Blender → FBX → import Unreal. Vérifier dans les exécutables disponibles la version réelle de Blender et du moteur ; le numéro dans un document ne suffit pas. Dans Blender, prouver une exportation de référence qui mesure 100 cm dans Unreal ; régler unités/transforms de façon reproductible. Normales extérieures, UV sans chevauchement pour les surfaces qui l'exigent, collisions simples et pivots cohérents. Les UV volontairement empilés doivent être documentés, sans contredire les règles du projet.

Pour une créature : silhouette approuvée, topo déformable, squelette stable, skinning, clips, sockets, PhysAsset, AnimBP, notifies, bounds, LOD, ombres. Pour une pièce modulaire : dimensions/connecteurs, collision, canal trace, pivot de grille, ISM/HISM et matériau partagé. Normal map DirectX conforme à l'import ; sRGB pour couleur, linéaire pour données.

Les imports/générateurs doivent proposer dry-run et liste d'assets concernés. Un mode force ne doit pas écraser tous les réglages d'artiste. Comparer source/hash/version pour reconstruire ce qui a changé ; journaliser les assets créés/modifiés/ignorés. Faire tourner les générateurs dans une copie isolée quand ils n'ont pas encore cette garantie.

Registre recommandé : `asset_id, owner, source_url, author, license, source_file, export_file, unreal_path, generator_version, status, evidence`. Les achats externes restent une décision distincte ; aucune banque particulière n'est présentée ici comme gratuitement exploitable sans lecture de licence.

## Performance : budgets mesurés

Choisir une machine cible réelle puis mesurer la build packagée. Point de départ : 1080p, preset Medium, objectif 60 FPS (16,7 ms/frame), temps de frame p95 et p99, mémoire CPU/GPU, stutters de shader, durée de génération et réseau. Un objectif n'est pas une certification. Documenter GPU/CPU/RAM/driver, seed, joueurs, version, résolution interne et upscaler.

La [documentation Epic Lumen](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-performance-guide-for-unreal-engine) distingue budgets et niveaux de qualité : effectuer des comparaisons de presets avant de choisir éclairage et cible matérielle. Ne pas déclarer une GTX compatible/60 FPS à partir d'un fichier ini.

Scènes de mesure : hub, vue de longue portée, salle à éclairages nombreux, poursuite avec quatre joueurs, génération/shift, Poolrooms quand disponible. Profil CPU/GPU et Unreal Insights ; ne pas optimiser au hasard en réduisant tout. Examiner ombres, nombre de lumières, transparence eau/brume, coût des traces et Tick, chargements synchrones, navigation et réplication.

ISM obligatoires pour sols/cloisons/néons ; contrôler les variations par données d'instance, matériaux partagés. Éloignement : réduire fréquence perception/animation/sons selon pertinence sans retirer un indice nécessaire. Les réglages Low ne doivent pas supprimer l'eau, les indices de menace ou donner une vue à travers le blé. Budget mémoire, triangles et textures à fixer après profilage d'une scène représentative, pas par quotas universels.

## QA qui prouve quelque chose

La [documentation Epic Automation](https://dev.epicgames.com/documentation/unreal-engine/automation-test-framework-in-unreal-engine?lang=en-US) distingue plusieurs catégories de tests. Séparer dans ce projet les contrôles de présence, tests unitaires, tests de contenu, scénarios gameplay et validation humaine.

| Niveau | Exemple | Condition de réussite |
|---|---|---|
| Statique | Chemin asset, type, référence, config | Aucun chemin indispensable manquant ; rapport nominatif |
| Unitaire | Quota, coûts, distances, état d'attaque | Résultat attendu et cas limites |
| Contenu | LoadPackage, mesh, AnimBP, sockets, cook | Chargement réel, dépendances et ressources requises |
| Procédural | Campagne de seeds, graphes et NavMesh | 1 000 seeds proposées par configuration ; chemin objectif/sortie, 80 m, aucune clé inaccessible ; seed fautive sauvegardée |
| Fonctionnel réseau | Porte, pickup simultané, soin, revive, extraction | Serveur et clients cohérents ; pas de duplication ni mutation invalide |
| Visuel/audio | Silhouette, éclairage, attaque, occlusion | Captures/écoutes sur build identifiée et checklist indépendante |
| Performance | Run représentative | Mesures sous budgets convenus ; stutters documentés |
| Release | Installation propre, lancement, travel, retour, fermeture | Development/Shipping sans avertissements selon AGENTS ; exécutable autonome rendu, sans console/debug |

Ne pas lancer `Run_Auto_Check.ps1` complet dans une session de travail sans examiner ses effets : il invoque l'orchestrateur. Ne pas utiliser la fermeture globale d'éditeurs du packaging. Ne pas compter un test interrompu, ignoré ou non trouvé comme réussi. Timeout explicite ; sortie non nulle si test absent ou erreur. Génération d'assets hors des SmokeFilter et des tests de lecture seule.

Rapport de tâche recommandé : commit de base/final, working tree propre ou delta documenté, moteur/toolchain, commande exacte, cas attendus/découverts/exécutés/pass/fail/skip, résultat de chaque critère, emplacement des preuves, limites, réviseur. Aucun « 100 % » général : nommer la portée validée.

## Organisation des agents

Une spécification active précise capacités jouables et différées ; les anciens documents deviennent historiques avec liens. Un coordinateur maintient le graphe de dépendances et les décisions, chaque spécialiste possède un périmètre de fichiers, le Sentinel vérifie une tâche achevée contre des critères explicites. La revue architecte précède tout changement architectural ; la revue Blender couvre nouveaux objets/entités.

Les fichiers binaires Unreal et un même .umap ont un seul éditeur à la fois. Réserver les fichiers partagés (Character, Generator, GDD) ; paralléliser seulement des tâches indépendantes. Une branche par tâche, commits petits, base Git identifiée. Ne pas intégrer toutes les modifications locales avec `git add .`. Pas de reset/clean pour résoudre la concurrence.

Regrouper scripts progressivement dans `scripts/generation`, `scripts/import`, `scripts/diagnostics`, `scripts/onetime` en mettant à jour les références. Le dossier `scripts/` existe déjà. Étudier Git LFS pour les nouveaux binaires et conserver les sources ; toute migration de l'historique constitue un chantier séparé, pas un nettoyage automatique.

## Jalons et décisions de poursuite

| Jalon | Travail | Preuve qui autorise la suite |
|---|---|---|
| M0 — vérité | Inventaire, conflits de spec, QA sûre, reproduction P0 | Aucune confusion prévu/implémenté/validé ; runner fiable |
| M1 — run fiable | Interactions, transactions, mêlée, extraction | Hôte + client jouent une boucle sans blocage |
| M2 — run lisible | Hound fini, Level 0, menus, son et tutoriel | Nouveaux joueurs comprennent danger et retour |
| M3 — slice | Smiler puis Clump si justifié, cinq outils, progression courte | Plusieurs groupes souhaitent retenter ; pas d'échec technique critique |
| M4 — alpha | Level 1/37, sessions Internet, sauvegarde robuste | Variation utile et performance maintenue |
| M5 — publication | Démo, store assets réels, support et release | Package indépendant vérifié, capacités annoncées démontrées |

Pas de calendrier chiffré de développement avant estimation par tâche sur le matériel disponible. Les agents accélèrent la production, mais les captures, tests multijoueurs, arbitrages artistiques et playtests restent des passages obligatoires.

## Publication et maintenance

Préparer une page Steam quand les captures montrent le jeu réel et que la promesse est stable ; ne pas annoncer onze biomes finis. Trailer : montrer interaction, danger et retour, sans séquence IA présentée comme gameplay. La [documentation wishlist de Steam](https://partner.steamgames.com/doc/marketing/wishlist) sert de base au suivi ; aucune garantie algorithmique de seuil retenue.

Démo courte, formulaire seed/build/reproduction, canal bugs, notes de version et sauvegardes compatibles. Avant release : crédits/licences, langues effectivement disponibles, exigences matérielles mesurées, politique de voix, configuration publique sans secrets, crash logs sans contenu vocal. Après release : classer crash/perte de progression avant équilibrage, proposer rollback de build et schéma save compatible. Ne pas multiplier événements et nouveaux monstres pendant une régression critique.
