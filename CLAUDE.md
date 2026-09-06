# M.E.G. : RECLAMATION — Guidelines MCP & Architecture Système

> Instructions système pour les agents MCP Blender, Unreal Engine et le développement du projet.

---

## 1. Principes Communs aux deux MCP

1. **Un appel = une opération.** Ne jamais empiler plusieurs actions dans un seul appel d'outil. Une géométrie, un matériau, un déplacement, une modification de Blueprint à la fois. Les opérations complexes se décomposent en séquence, pas en un seul prompt fourre-tout.
2. **Vérifier l'état avant d'agir.** Avant toute opération sur l'éditeur, interroger l'état courant (statut de connexion, hiérarchie de la scène/du niveau, objets existants) plutôt que de supposer un contexte.
3. **Nommer explicitement.** Toujours référencer les objets/assets par leur nom exact et unique. Ne jamais utiliser de références floues ("l'objet à côté de", "le mesh principal"). Si un nom n'existe pas encore, le définir avant de l'utiliser dans des appels suivants.
4. **Lire chaque résultat avant de continuer.** Ne pas enchaîner les appels sans valider le retour du précédent (succès, erreur, état réel de la scène). En cas d'échec, diagnostiquer avant de retenter.
5. **Pas d'appels concurrents/parallèles.** Les serveurs MCP Blender et Unreal exécutent les commandes de façon séquentielle sur le thread principal de l'éditeur. Envoyer des appels en parallèle peut geler l'éditeur ou provoquer des deadlocks. Un appel, on attend le résultat, puis le suivant.
6. **Exception pour les opérations répétitives homogènes.** Pour des boucles de 5+ opérations identiques (batch rename, spawn multiple, assignation de matériaux par pattern de nom), regrouper via un script exécuté côté serveur plutôt que multiplier les appels un par un.
7. **Séparer setup/nettoyage (agent) et décisions créatives fines (humain).** L'agent excelle sur le travail structurel et répétitif : blockout, lighting de base, rigging boilerplate, batch operations, rapports de scène. Le raffinement créatif (sculpt de détail, edge flow, proportions finales, tuning artistique) reste un travail manuel — l'agent fournit les 80%, l'humain fait les 20% qui comptent.
8. **Toujours donner les valeurs complètes en un seul prompt** quand des paramètres interagissent entre eux (ex : focale + ouverture + distance de mise au point en caméra, largeur + segments pour un bevel). Ne pas les fixer par petites touches successives : ça introduit des incohérences.
9. **Demander un plan avant une action large.** Pour toute modification touchant plusieurs objets/systèmes, faire décrire le plan d'action par l'agent avant exécution, et valider ce plan avant de laisser l'agent l'exécuter.

---

## 2. Blender MCP — Spécifique

### Workflow recommandé (par passes, jamais one-shot)
Blockout → test d'échelle → matériaux → lighting → caméra → nettoyage → render → itération. Ne jamais demander une scène complète finalisée en un seul prompt.

### 4 Passes Strictes :
- **Passe 1 — Échelle et blocage** : Unités métriques (1 unité = 1 mètre dans Blender), volumes simples et caméra.
- **Passe 2 — Éclairage de base** : Poser l'ambiance lumineuse avant les détails.
- **Passe 3 — Raffinement des formes** : Bevel, Subdivision Surface, Solidify.
- **Passe 4 — Matériaux et textures** : Nœuds Principled BSDF (Roughness, Metallic, Bump/Normal procédural).

### Rigueur Technique dans le Code Python Généré (`bpy`)
- Toujours appliquer les transformations : `bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)` après avoir redimensionné un objet.
- Ranger chaque objet dans des Collections dédiées (`Lighting`, `Architecture`, `Props`, `Bestiary`) et nommer précisément chaque mesh et matériau.
- Placer l'origine à la base des objets posés au sol.

### Matériaux
Pour tout setup de normal map ou node graph shader, toujours préciser : nom du fichier texture, color space (Non-Color pour normal/roughness), le node intermédiaire requis (ex. Normal Map node), et la connexion cible exacte.

---

## 3. Unreal Engine MCP — Spécifique (UE 5.8+)

1. **Vérifier le statut de connexion** en premier via les outils MCP Unreal.
2. **Consulter la documentation/API** avant d'écrire du code C++ ou Blueprint (APIs UE 5.8).
3. **Priorité aux skills de projet** : un skill de projet spécifique prime toujours sur les instructions génériques.
4. **PCG (Procedural Content Generation)** : workflow reference-driven, inspecter les graphes existants, sélectionner explicitement les assets, exécuter par étape incrémentale.
5. **Debug méthodique** : inspecter l'état (collisions, physique, hiérarchie) avant de proposer un correctif.
6. **Sécurité & approbation** : validation préalable pour toute suppression d'asset ou modification de configuration projet.

---

## 4. Check-list Rapide avant de Lancer un Agent 3D

- [ ] Statut de connexion MCP vérifié (Blender et/ou Unreal)
- [ ] Objets/assets concernés nommés explicitement
- [ ] Tâche découpée en étapes unitaires, pas en un seul prompt global
- [ ] Un plan validé pour toute modification touchant plusieurs éléments
- [ ] Pas d'appels MCP parallèles/concurrents
- [ ] Résultat de chaque appel lu avant de continuer
- [ ] Décision créative fine réservée à l'humain, pas déléguée à l'agent
