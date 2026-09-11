# Périmètre actif — M.E.G. : Reclamation

Ce fichier suit l'implémentation du [plan du 10 septembre](audit-2026-09-10/README.md). Il distingue les objectifs de production des capacités effectivement prouvées. AGENTS.md conserve ses exigences techniques ; les anciens rapports de complétion ne valent pas preuve de release.

## Objectif immédiat

Une boucle hub → Level 0 → fusible/loot → extraction → hub, jouable seul puis en coop, avec un Hound fini. Smiler puis Clump suivent après validation de cette base. La disponibilité actuelle de onze entrées de biomes n'est pas une certification de onze biomes finis.

## Décisions actives

- Smiler : lumière ou regard direct avec ligne de vue déclenche charge, selon AGENTS.md. La paralysie par regard sombre est supprimée. Adaptation M.E.G., pas canon universel des Backrooms. Le GDD est aligné sur cette règle ; réglage et présentation restent à tester.
- Dégâts : une attaque autorisée ouvre une seule fenêtre d'impact. Le fallback des entités sans montage reste provisoirement une trace unique, jamais un dégât direct à travers un obstacle. La finition de l'animation demeure nécessaire.
- Génération : la distance minimale spawn → extraction est 80 m. Connexité abstraite et navigation effective sont deux validations distinctes.
- Pause : vraie pause solo ; menu réseau sans arrêt du monde, avec indication explicite.
- Scope : aucune nouvelle entité, banque payante ou migration destructive d'assets avant preuve de la boucle de base.
- Nouvelle campagne : réinitialisation uniquement après confirmation explicite ; la progression des invités et la reconnexion restent à fiabiliser.
- Hound : perception sonore réelle par stimuli localisés (distance avec atténuation, occlusion par portes/murs, intensité perçue, horodatage et mémoire bornée). Hystérésis de cible pour empêcher l'oscillation. Règle du regard (Manuel M.E.G. Entité 8) : fixer directement le Hound (< 4m, ligne de vue directe, cône de 45°) l'intimide et suspend sa charge ; briser le regard relance la poursuite. Rupture de poursuite bornée vers la dernière position connue, puis recherche locale bornée avant retour au calme. Les leurres sonores attirent le Hound à leur position exacte.

## Matrice de maturité au lancement des corrections

| Élément | Présence constatée | Preuve restant nécessaire |
|---|---|---|
| Hub / Level 0 / extraction | Maps et code présents | Run complète rendue, solo puis client distant |
| Bestiaire | Classes, BT/BB et assets de substitution présents | Chargement des classes effectives, rig/animation/silhouette, rencontre jouée |
| Interactions | Implémentations C++ | Vérification multi-processus et refus des actions invalides |
| Procgen | Layout, ISM, tests abstraits | Navigation après décor/portes, seeds et absence de blocage |
| Audio/VoIP | Code et assets, travail local antérieur | Écoute spatiale et transmission entre deux machines |
| Menus/options | HUD Canvas et persistance de réglages | Chaque action et application réelle des réglages, manette/localisation |
| Réseau/sauvegarde | Systèmes partiels | Internet, reconnexion et intégrité de la progression |
| QA | 36 déclarations dans la suite actuelle | Résultats effectifs dans le suivi de livraison, builds et package |

Voir [l'état de reprise](ETAT_REPRISE.md) pour les correctifs, commandes exécutées et résultats de cette session. Ne pas modifier une ligne en « terminé » sans sa preuve associée.
