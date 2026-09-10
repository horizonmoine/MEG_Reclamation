# Sources et portée de la recherche

Consultation : 10 septembre 2026. Les pages web sont évolutives. Les trois textes joints par le propriétaire sont des documents de travail, pas des sources indépendantes ni des observations de playtest. L'audit utilise lecture de fichiers, Git, recherche web et revues spécialisées architecte/Sentinel. Aucun accès visuel à l'éditeur, écoute de run ou commande MCP Unreal/Blender n'a été revendiqué.

## Références consultées

| Source primaire | Ce qu'elle étaye | Utilisation dans le plan |
|---|---|---|
| [Dépôt du jeu](https://github.com/horizonmoine/MEG_Reclamation/tree/main) | Projet désigné par le propriétaire ; remote Git confirmé | Base locale 98961b3 plus modifications préexistantes ; diagnostic vérifiable par fichiers |
| [Lethal Company — présentation du développeur sur Steam](https://store.steampowered.com/app/1966720/Lethal_Company/) | Coop, collecte de ferraille, quota, outils et coordination ; description du jeu | Comparer décisions de risque, retour et entraide ; aucun chiffre de ventes ni causalité de succès repris |
| [Escape the Backrooms — Steam](https://store.steampowered.com/app/1943950/Escape_the_Backrooms/) | Exploration horrifique coop 1–4, niveaux et évitement de dangers | Comparer variété des lieux et objectif de traversée ; ne pas transposer ses cartes ou assets |
| [Valve : The AI Systems of Left 4 Dead, Mike Booth](https://cdn.akamai.steamstatic.com/apps/valve/2009/ai_systems_of_l4d_mike_booth.pdf) | Référence technique publiée du directeur et de la population IA | Inspiration de rythme ; budget et phases de M.E.G. restent des propositions |
| [Epic — Lumen Performance Guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-performance-guide-for-unreal-engine) | Budgets et arbitrages de qualité Lumen | Mesures par presets plutôt que prédiction de performances depuis ini |
| [Epic — Automation Test Framework](https://dev.epicgames.com/documentation/unreal-engine/automation-test-framework-in-unreal-engine?lang=en-US) | Tests unitaires, fonctionnalités et stress de contenu | Séparer catégories et preuves |
| [Epic — Run Automation Tests](https://dev.epicgames.com/documentation/unreal-engine/run-automation-tests-in-unreal-engine) | Exécution des tests et contexte Unreal | Vérifier découverte/exécution avec moteur réel ; rapport nominatif |
| [Game Accessibility Guidelines — Basic](https://gameaccessibilityguidelines.com/basic/) | Reconfiguration, confort, lisibilité et alternatives aux signaux uniques | Critères concrets de menus et options ; valeurs pixel/durée proposées par cet audit |
| [Steamworks — User Reviews](https://partner.steamgames.com/doc/store/reviews) | Cadre du système d'avis Steam | Distinguer avis, métriques de boutique et validation du jeu |
| [Steamworks — Wishlists](https://partner.steamgames.com/doc/marketing/wishlist) | Fonctionnement et suivi des listes de souhaits | Pas de seuil garanti d'algorithme ni calendrier marketing universel |
| [Backrooms Wikidot — Entity 3, Smilers](https://backrooms-wiki.wikidot.com/entity-3/section) | Cette version décrit attraction lumineuse et maintien du regard pour retraite prudente | Rend visible le conflit avec AGENTS ; adaptation M.E.G. explicitement nommée |
| [Backrooms Wikidot — Entity 8, Hound](https://backrooms-wiki.wikidot.com/entity-8) | Cette version décrit quadrupédie humanoïde et intimidation temporaire par regard | Pas de transformation de ce passage en immobilisation permanente universelle |
| [Backrooms Wikidot — Level 0](https://backrooms-wiki.wikidot.com/level-0) | Une version communautaire du lieu, actuellement intitulée Threshold | Ne pas confondre titre du jeu, version wiki et canon universel |
| [Backrooms Wikidot — Licensing Guide](https://backrooms-wiki.wikidot.com/licensing-guide) | Attribution et partage à l'identique, distinctions pour images et autres contenus | Registre des œuvres effectivement reprises, vérification par élément |

## Lore : conclusion exploitable

Définir un **canon M.E.G. de gameplay** qui cite ses inspirations et versions. Les directives actuelles du projet sur le Smiler ne sont pas identiques à la page Wikidot consultée : le plan suit la contrainte locale en tant qu'adaptation, et demande d'aligner ensuite GDD, code et manuel. Les règles proposées pour les autres monstres ne sont pas présentées comme un relevé encyclopédique.

Pour les reprises de contenu Wikidot, le guide indique CC BY-SA 3.0 avec attribution et obligations de partage à l'identique ; certaines ressources ont des conditions distinctes. Créditer chaque œuvre et auteur réellement repris, vérifier les images séparément et examiner le périmètre des adaptations avant distribution. Cela ne permet pas d'affirmer sans analyse que toute ligne du moteur ou du code propriétaire devient automatiquement soumise à cette licence. Ce document n'effectue pas une autorisation juridique de distribution.

Auteurs affichés dans les pages consultées : Smilers, texte original attribué à Palico22 et réécriture à Stretchsterz ; Hound, Palico22 et réécriture à 1000dumplings. Ces attributions aident le futur registre, elles ne remplacent pas l'inventaire des contenus effectivement incorporés au jeu.

## Attentes joueurs : ce qui est connu et ce qui est hypothèse

Les pages de jeu décrivent des promesses de produit. Elles ne constituent pas une enquête représentative des attentes. Aucun corpus systématique d'avis n'a été codé ici ; aucun pourcentage de satisfaction des joueurs de M.E.G. n'est inventé. Lisibilité, stabilité, contre-jeu et confort sont des recommandations fondées sur le diagnostic et les guides ; leur importance relative sera mesurée en playtest.

Pour un complément d'étude, échantillonner séparément avis positifs/négatifs, récents/anciens, français/anglais et solo/coop quand identifiable ; conserver liens, date, temps de jeu, version mentionnée et thèmes. Distinguer problème récurrent, effet d'une mise à jour et préférence personnelle. Ne pas transformer une anecdote de critique en exigence universelle.

R.E.P.O., GTFO et Pacific Drive sont évoqués dans les documents joints. Leurs statistiques, histoires de production et promesses commerciales n'ont pas été revérifiées exhaustivement dans cette passe et ne sont donc pas utilisées comme preuves. La recherche principale porte sur les deux références explicitement demandées, le lore, l'accessibilité, le rythme et les outils de validation.
