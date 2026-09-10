# M.E.G. : Reclamation — audit et plan de production

Date : 10 septembre 2026. Base Git : `98961b3a2160eed376df3683491bdf01bf448569`, branche `main`, identique au distant au début de l'audit. Le dossier local contient également des modifications en cours : les constats concernent cet état local, pas exclusivement le commit publié.

## Mon évaluation

Le concept possède une direction intéressante : récupérer des ressources dans un environnement où chaque outil de survie peut devenir un risque. Le dépôt contient une véritable implémentation C++, des systèmes procéduraux et des tests. Il faut maintenant démontrer une expédition complète, compréhensible et fiable à plusieurs joueurs. L'étendue du code et le nombre de cartes ne permettent pas de conclure à la qualité du jeu.

**Priorité recommandée : fiabiliser les interactions réseau et les dégâts, terminer une petite expédition au Level 0, puis améliorer son ressenti avec des joueurs.** Produire simultanément onze biomes avant cette validation multiplie les problèmes à corriger.

Je ne donne pas de note de plaisir, de peur ou de performance : aucune session de jeu rendue et écoutée, aucun playtest humain, aucune mesure GPU n'ont été réalisés dans cet audit. Les notes chiffrées de qualité seraient inventées. La maturité constatée est celle d'un prototype à nombreux systèmes, dont la qualité d'intégration reste à prouver.

## Lire et exécuter

1. [Diagnostic vérifié et correction des anciens audits](01_DIAGNOSTIC.md).
2. [Plan de design : boucle, menus, interactions, entités, biomes, son](02_DESIGN.md).
3. [Architecture, assets, performance, QA et production](03_PRODUCTION.md).
4. [Backlog ordonné et prompts à donner aux agents](04_PROMPTS_AGENTS.md).
5. [Sources et méthode de recherche](05_SOURCES.md).
6. [Résultats de validation de cette livraison](06_VALIDATION.md).

Les spécifications proposées ci-dessous sont un plan de refonte, pas des fonctionnalités déjà livrées. Le GDD et AGENTS.md n'ont pas été remplacés : les divergences, notamment Smiler et périmètre des biomes, sont des décisions explicites à traiter dans la première tâche. Les trois textes fournis ont servi de pistes, puis ont été recoupés avec le projet ; ils ne constituent pas des preuves indépendantes.

## Garder, modifier, différer, supprimer

| Décision | Contenu | Raison |
|---|---|---|
| Garder | Extraction sans armes, outils à contrepartie, retour au hub, bruit/lumière, FRandomStream, instancing | Base de l'identité et fondations réutilisables |
| Corriger immédiatement | Interaction des clients, validations RPC, dégâts mêlée, preuves QA | La confiance dans les règles passe avant la quantité |
| Finir | Hub compact, Level 0, Hound, lampe, loot, un puzzle, extraction | Une boucle complète permet des retours utiles |
| Reconcevoir progressivement | Menus et options, onboarding, rythme, progression et sanité | Lisibilité et décisions significatives |
| Différer | Infection, mimétisme vocal, inventaire Tetris, prestige, no-clip aléatoire létal, onze biomes simultanés | Coûts d'intégration élevés avant validation du cœur |
| Retirer du parcours publié | Boutons sans comportement réel, contenu non validé, proclamations de certification générale | Éviter les promesses trompeuses |
| Supprimer après audit de références | Assets de template inutilisés, plugins réellement inutiles, scripts obsolètes | Ne jamais casser les références Unreal par nettoyage aveugle |

## Première séance de travail

Donner le contrat commun et le prompt `P00` au coordinateur. Ensuite `P01` et `P02` à l'architecte, avec reproduction indépendante par le Sentinel. Le premier résultat recherché est simple : un client distant ouvre une porte, ramasse un objet, le dépose, subit un seul impact d'attaque et rentre au hub avec un état cohérent.
