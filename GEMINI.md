# M.E.G. : RECLAMATION — Directives Projet & Guidelines MCP (Antigravity)

Ce document constitue le **System Prompt & Guide Opérationnel** pour tous les agents intervenant sur le projet **M.E.G. : Reclamation** (Unreal Engine 5.8 C++, Blender 5.2.1 LTS, Google Antigravity SDK).

---

## 0. Règles C++ & Réseau OBLIGATOIRES (lire en premier)

Pour tout fichier sous `Source/`, applique intégralement :
- `.agents/rules/ue5_cpp_network_rules.md` : nommage Epic, `TObjectPtr`, includes minimaux, modèle 100 % Server-Authoritative, format de livraison.
- `docs/architecture/NETWORK_ARCHITECTURE.md` : matrice des responsabilités GameMode / GameState / PlayerState / Character / Subsystems.
- `docs/missions/00_PROTOCOL.md` : ordre des missions (`01` -> `06`) et porte de qualité (lint `scripts/ci/lint_ue_network.py`, build 0 warning, `Run_Automation_Tests.ps1`, `Run_Auto_Check.ps1`).

Rejet immédiat : float répliqué `*TimeRemaining`, Server RPC sur GameState/GameMode, pointeur brut `UPROPERTY`, `TArray<UObject*>`/`TMap` répliqués, pseudo-code ou `...` dans un fichier livré, arme joueur.

---

## 1. Principes Communs aux MCP (Blender & Unreal Engine)

1. **Un appel = une opération.** Ne jamais empiler plusieurs actions dans un seul appel d'outil. Une géométrie, un matériau, un déplacement, une modification de Blueprint à la fois. Les opérations complexes se décomposent en séquence, pas en un seul prompt fourre-tout.
2. **Vérifier l'état avant d'agir.** Avant toute opération sur l'éditeur, interroger l'état courant (statut de connexion, hiérarchie de la scène/du niveau, objets existants) plutôt que de supposer un contexte.
3. **Nommer explicitement.** Toujours référencer les objets/assets par leur nom exact et unique (`SM_Harmonic_Resonator`, `SK_Hound`, `Lvl_Hub_BaseAlpha`). Ne jamais utiliser de références floues ("l'objet à côté de", "le mesh principal"). Si un nom n'existe pas encore, le définir avant de l'utiliser dans des appels suivants.
4. **Lire chaque résultat avant de continuer.** Ne pas enchaîner les appels sans valider le retour du précédent (succès, erreur, état réel de la scène). En cas d'échec, diagnostiquer avant de retenter.
5. **Pas d'appels concurrents/parallèles.** Les serveurs MCP Blender et Unreal exécutent les commandes de façon séquentielle sur le thread principal de l'éditeur. Envoyer des appels en parallèle peut geler l'éditeur ou provoquer des deadlocks. Un appel, on attend le résultat, puis le suivant.
6. **Exception pour les opérations répétitives homogènes.** Pour des boucles de 5+ opérations identiques (batch rename, spawn multiple, assignation de matériaux par pattern de nom), regrouper via un script exécuté côté serveur plutôt que multiplier les appels un par un.
7. **Séparer setup/nettoyage (agent) et décisions créatives fines (humain).** L'agent excelle sur le travail structurel et répétitif : blockout, lighting de base, rigging boilerplate, batch operations, rapports de scène. Le raffinement créatif (sculpt de détail, edge flow, proportions finales, tuning artistique) reste un travail manuel — l'agent fournit les 80%, l'humain fait les 20% qui comptent.
8. **Toujours donner les valeurs complètes en un seul prompt** quand des paramètres interagissent entre eux (ex : focale + ouverture + distance de mise au point en caméra, largeur + segments pour un bevel). Ne pas les fixer par petites touches successives : ça introduit des incohérences.
9. **Demander un plan avant une action large.** Pour toute modification touchant plusieurs objets/systèmes, faire décrire le plan d'action par l'agent avant exécution, et valider ce plan avant de laisser l'agent l'exécuter.

---

## 2. Blender MCP — Pipeline 3D Discipliné

La différence entre un résultat médiocre et un rendu bluffant avec le MCP Blender tient à une règle simple : **ne jamais laisser l'agent modéliser à l'aveugle en un seul prompt.** Traiter l'agent comme un script-opérateur au sein d'un pipeline 3D structuré.

### Workflow en Passes Strictes (Jamais de One-Shot)
1. **Passe 1 — Échelle et blocage** : définir les unités métriques (1 unité = 1 mètre dans Blender, export à l'échelle 1:1 pour Unreal), créer les volumes simples (murs, sol, boîtes d'encombrement) et placer la caméra.
2. **Passe 2 — Éclairage de base** : poser l'ambiance lumineuse (lumière principale + débouchage, teintes néon 60Hz des Backrooms) avant même d'avoir les détails.
3. **Passe 3 — Raffinement des formes** : biseauter les arêtes (Bevel), appliquer les modificateurs (Subdivision Surface, Solidify, Displace).
4. **Passe 4 — Matériaux et textures PBR** : configurer les nœuds du Principled BSDF (Albedo, Rugosité, Métallique, Bump/Normal procédural ou textures importées).

### Boucle de Rétroaction Visuelle
- Exécuter des rendus rapides basse résolution (10 à 32 échantillons) ou des captures de viewport pour analyser l'image obtenue.
- Vérifier immédiatement l'orientation de la caméra, les normales inversées ou les objets flottants, et corriger les coordonnées.

### Rigueur Technique dans le Code Python Généré (`bpy`)
- **Appliquer les transformations** : toujours exécuter `bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)` après avoir redimensionné un objet, sinon les biseaux et les textures s'étirent.
- **Nommer et classer** : ranger chaque objet créé dans des collections dédiées (`Lighting`, `Architecture`, `Props`, `Bestiary`) et nommer précisément chaque mesh et chaque matériau (`SM_...`, `SK_...`, `M_...`, `MI_...`).
- **Origine des objets** : s'assurer que le point d'origine est placé à la base des objets posés au sol plutôt qu'au centre géométrique.

---

## 3. Unreal Engine MCP — Spécifique (UE 5.8+)

1. **Vérifier le statut de connexion** en premier via les outils MCP Unreal.
2. **Consulter la documentation/API** avant d'écrire du code C++ ou Blueprint (les APIs UE 5.8 évoluent rapidement).
3. **Priorité aux skills de projet** : un skill ou toolset configuré au niveau projet prévaut sur les règles génériques.
4. **PCG (Procedural Content Generation)** : workflow reference-driven, inspecter les graphes existants, sélectionner explicitement les assets, exécuter par étape incrémentale.
5. **Debug méthodique** : inspecter l'état (collisions, physique, hiérarchie) avant de proposer un correctif ; pas de fix à l'aveugle.
6. **Sécurité & approbation** : validation préalable pour toute suppression d'asset ou modification de configuration projet (`Config/*.ini`, `*.uproject`).

---

## 4. Cadre Canonique du Jeu (GDD M.E.G. : Reclamation)

- **Lore & Factions** : M.E.G. (Major Explorer Group), Basement Complex (Niveau 1, Base Alpha), B.N.T.G. (cartel marchand de troc), The Insurrection (dissidents armés du Niveau 3).
- **Mandat Logistique & Quota Tri-Journalier (Cycles de 3 Jours)** :
  $$Q(k, N) = \lfloor Q_{\text{base}} \cdot (1 + \alpha)^{k-1} + \beta \cdot (k-1)^{1.4} \rfloor + \delta \cdot (N - 1)$$
  avec $Q_{\text{base}} = 180$, $\alpha = 0.32$, $\beta = 55$, $\delta = 45$, $N \in [1, 4]$.
  - Rapatriement de Dog Tag d'agent mort : $+50\text{ BR}$ d'assurance funéraire.
  - Cadavre abandonné sur le terrain : $-80\text{ BR}$ de pénalité militaire.
- **Console Asymétrique au Sas B.R.C.** :
  - Radar topologique à balayage 2 Hz (triangles verts = agents, points rouges = entités, losanges jaunes = scrap).
  - Commandes CLI : `SWITCH`, `RADAR`, `DOOR`, `OVERLOAD`, `VENT`, `PING`, `SCAN`, `BROADCAST`.
  - Batterie du Sas : réserve de $1000\text{ Wh}$ (maintien porte $4\text{ Wh/s}$, overload $150\text{ Wh}$).
- **Physique Corporelle & Endurance** :
  - Décroissance d'endurance : $\frac{dS}{dt} = - C_{\text{base}} \cdot (1 + M_{\text{charge}}/M_{\text{ref}})^\gamma$ avec $C_{\text{base}} = 10$, $M_{\text{ref}} = 20\text{ kg}$, $\gamma = 1.45$.
  - Rupture cardiorespiratoire : 4 secondes d'arrêt net avec halètement de 85 dB audible à 30 mètres par les entités.
  - Objets à deux mains : aucun outil brandi, BoxCast préventif anti-clipping.
- **Traque Acoustique RMS (Hound)** :
  $$A_{\text{RMS}} = \sqrt{\frac{1}{N} \sum_{n=1}^N x[n]^2} > 0.03 \implies \text{Alerte et traque immédiate}$$
- **Contre-mesures Bestiaire** :
  - *Smiler* : éteindre les torches, contact visuel fixe, recul lent sans tourner le dos.
  - *Hound* : silence radio absolu, accroupissement, contact visuel frontal d'intimidation à moins de 4 m.
  - *Skin-Stealer* : vérification biométrique radar sas (pas de signal valide) ou mot de passe oral.
  - *Partygoer* : détection des commandes avec `:)`, neutralisation au désodorisant de réalité.
- **Procédural 3D WFC** : cellules 6×6×4 m, minimisation de l'entropie de Shannon, validation topologique A* avec seuil d'accessibilité du scrap $\ge 85\%$.
