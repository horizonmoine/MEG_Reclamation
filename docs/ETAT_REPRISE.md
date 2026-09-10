# État de reprise — 11 septembre 2026

Base de ce lot : `577e1aa` (audit et plan directeur déjà publiés). Le jeu reste un prototype en consolidation. Ce document remplace les anciennes annonces de validation globale, qui n'étaient pas étayées par une exécution complète.

## Correctifs intégrés

- **Interactions** : intention envoyée par le personnage possédé, portée et cible recalculées côté serveur. Les mutations directes de poids, sanité, infection et réanimation ne sont plus des RPC accessibles au client. Réanimation conditionnée à la proximité, la visibilité et trois secondes de maintien des conditions.
- **Cachettes et conduits** : entrée et demande de sortie traitées côté serveur ; vérification de l'occupant réel. Les flags du joueur suivent la fin des transitions et les sorties automatiques, et sont répliqués au propriétaire.
- **Mêlée** : une seule fenêtre de dégâts par attaque ; notifies répétés ignorés ; ligne de vue et états de la cible vérifiés. Le fallback sans montage utilise une trace, sans dégâts directs supplémentaires. L'animation finale reste à produire et vérifier.
- **Smiler** : regard ou lumière avec visibilité déclenchent la charge ; suppression de la paralysie par regard dans le noir ; retour au comportement normal après perte du stimulus. Règle de cette adaptation, conformément à AGENTS.md.
- **Procgen classique** : sélection d'une extraction à au moins 80 m, selon chemin cardinal et distance directe ; essais bornés puis layout de secours soumis aux mêmes contraintes. Échec explicite sans remplacer un layout valide. Cette preuve abstraite ne valide pas la navigation après placement des meshes et portes.
- **Apparition des entités** : filtre de distance de 35 m pour les placements Hound, Smiler et Clump du générateur classique. Les contraintes de visibilité, les autres espèces et les vagues tardives restent à vérifier.
- **Porte et pause** : fermeture stable avec grands pas de temps ; angle simulé serveur. Pause effective en solo ; en réseau, le menu prévient que le monde continue. Anti-rebond de l'interface indépendant du temps du monde.
- **Nouvelle campagne** : confirmation explicite avant réinitialisation, annulée par Échap ou navigation. Aucun effacement dès la première sélection.
- **Hub** : présence de terminal, sas et dépôt assurée par le code de secours ; nettoyage limité aux classes d'entités hostiles et de générateur connues. Aucun nettoyage des décors par nom « Cube ».
- **Sanité** : suppression d'un second appel par frame au calcul de pression mentale.
- **QA** : tests séparés du lancement automatique ; suppression des appels de génération/import dans la suite ; contrôle nominatif des résultats, erreurs, tests absents, doublons et code de sortie réel. Packaging sans fermeture forcée des éditeurs ouverts.

Les changements locaux préexistants des déplacements, du Hound, des transitions de map et de restauration absolue des ressources ont également été relus et intégrés au lot source. Ils ne constituent pas une preuve de finition des fonctionnalités correspondantes.

## Vérification de cette livraison

Les résultats définitifs sont consignés ici à la fin de la validation. Journaux locaux : `Saved/Validation_2026_09_11/` et rapports moteur : `Saved/AutomationReports/`.

- Revue C++ par `ue_cpp_architect` : correctifs ciblés acceptés ; réserves réseau et hub documentées ci-dessous.
- Contrôles structurels : **27/27**. Il s'agit de présence de fichiers et d'outils, pas de 27 scénarios joués.
- Runner hors moteur : **8/8 scénarios conformes**, rapport complet accepté et rapports incomplets/dupliqués/échoués/ignorés/vides ou code processus non nul refusés. Trois scripts PowerShell syntaxiquement valides.
- Suite déclarée : **36 tests**, dont les six contrôles procgen inclus dans le filtre par défaut. Les rapports anciens à 30 tests ne couvrent pas cette version.
- Un premier build a échoué pendant les modifications d'en-têtes. Une nouvelle compilation du code stabilisé est nécessaire ; ce premier journal n'est pas une preuve de réussite.
- Les anciens rapports annonçant 30 succès comportaient des erreurs `LogAutomationTest` hors cas individuels. Ils ne sont pas acceptés comme validation globale.

## Ce qui reste à faire

1. **Boucle jouable** : parcourir hub → Level 0 → collecte/fusible → extraction → hub en rendu réel, puis avec deux processus et un client distant. Vérifier réanimation, décès, fin de mission, sortie des cachettes, portes et sauvegarde.
2. **Terminal en coop** : déplacer achats et autres intentions vers un acteur possédé ; plusieurs RPC du terminal restent sur un acteur non possédé par le client. La sécurisation de `ServerInteract` ne résout pas ce point.
3. **Reconnexion** : `PlayerState.GetPlayerId()` et le repli sur pseudo ne constituent pas une identité persistante authentifiée. Refaire l'identification, expiration et reprise du pawn en stase ; tester absence de duplication de ressources. D11 de l'audit reste ouvert.
4. **Hound** : le champ `CurrentAcousticRMS` n'a pas encore de producteur audio dans ce lot. Relier proprement la perception sonore serveur, BT/BB, déplacement et attaque animée. Vérifier le Hound réellement instancié et l'occlusion ; aucune certification de VoIP.
5. **Navigation** : tester le NavMesh après instanciation, portes et décoration ; vérifier objectifs et extraction sur de nombreuses graines. Étendre les garanties au générateur modulaire distinct. Le test modulaire historique ne prouve pas la règle des 80 m.
6. **Assets et hub** : inspecter les modifications locales de maps, audio et FBX, leurs exports/imports et leur rendu avant publication. Vérifier collision et placement du terminal/sas/dépôt ; la revue source ne remplace pas cette inspection.
7. **Menus, options et accessibilité** : application réelle de chaque réglage, remapping, manette, navigation souris, lisibilité, localisation et retours d'erreur. Une confirmation textuelle n'est pas une refonte complète du menu.
8. **Release** : reconstruire un package à partir du commit publié, contrôler ses dépendances, son lancement normal et une partie complète sans console. Aucun ancien exécutable local ne certifie ce lot.

Pour la suite, utiliser les prompts détaillés et leurs critères de réception dans le [plan directeur](audit-2026-09-10/README.md). Priorité à une boucle Level 0 fiable avant d'étendre les onze biomes.

## Reprendre sans perdre les travaux locaux

- Les snapshots antérieurs sont dans `Saved/Implementation_2026_09_10/preexisting.patch` et `preexisting-status.txt`. La version précédente de ce suivi est conservée dans `Saved/Validation_2026_09_11/reprise-before-verification.md`.
- Les configurations MCP personnelles, scripts exploratoires et modifications binaires non vérifiées restent locaux, hors du commit source. Ne pas utiliser `git reset --hard`, `git clean` ou un ajout global.
- Relancer l'éditeur après compilation avant les tests. Ne pas utiliser `MEG.BuildFullGame` comme simple contrôle : cette commande génère et modifie le contenu.

```powershell
./Run_Automation_Tests.ps1 -TimeoutSeconds 900
./Run_Auto_Check.ps1 -Mode Structural
```

Le mode `-ValidateReportPath` contrôle un rapport existant ; il n'exécute pas Unreal. Le mode `ReleaseSmoke` ne rend aucune image et ne certifie pas la jouabilité.