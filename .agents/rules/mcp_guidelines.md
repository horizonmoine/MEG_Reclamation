# Guidelines MCP — Blender & Unreal Engine

Ce document s'applique à tous les agents Antigravity utilisant les serveurs MCP Blender et Unreal Engine.

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

## 2. Blender MCP — Workflow en 4 Passes Strictes

Ne jamais modéliser à l'aveugle en un seul prompt.
1. **Passe 1 — Échelle et blocage** : Unités métriques, volumes simples, caméra.
2. **Passe 2 — Éclairage de base** : Ambiance lumineuse de base avant les détails.
3. **Passe 3 — Raffinement des formes** : Bevel, Subdivision Surface, Solidify.
4. **Passe 4 — Matériaux et textures PBR** : Nœuds Principled BSDF, roughness, normal map.

### Rigueur Technique du Code Python (`bpy`) :
- Appliquer les transformations après redimensionnement : `bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)`.
- Nommer et classer dans des Collections dédiées (`Lighting`, `Architecture`, `Props`, `Bestiary`).
- Point d'origine à la base des objets posés au sol.

---

## 3. Unreal Engine MCP — Spécifique (UE 5.8+)

1. Vérifier le statut de connexion du plugin en premier.
2. Consulter la documentation et les patterns d'API avant d'écrire du C++ ou Blueprint.
3. Priorité absolue aux Agent Skills du projet MEG_Reclamation.
4. PCG reference-driven et supervisé.
5. Debug analytique avec diagnostic préalable affiché avant tout correctif.
