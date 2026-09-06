# M.E.G. : RECLAMATION — Antigravity Multi-Agent Squad & Architecture

Ce document définit les rôles, directives et spécialisations des agents autonomes opérant sur le projet **M.E.G. : Reclamation** (Unreal Engine 5.8 C++).

---

## 1. Cartographie des Agents Spécialisés

### 1. `ue_cpp_architect`
- **Domaine** : Moteur Unreal Engine 5.8 C++, compilation MSVC 14.44, architecture gameplay, réseau & réplication RPC.
- **Règles strictes** :
  - Respect scrupuleux des conventions Epic Games (préfixes `A`, `U`, `F`, `E`, `S`).
  - Utilisation de `TObjectPtr<T>` pour les membres UPROPERTY, `TWeakObjectPtr<T>` pour les références sans ownership.
  - Vérification systématique des pointeurs (`if (!Obj) return;`) avant déréférencement.
  - Zéro avertissement de compilation en `Development Win64` et `Shipping Win64`.

### 2. `procgen_designer`
- **Domaine** : Génération procédurale des couloirs et salles liminales, algorithmes WFC (Wave Function Collapse) et BSP.
- **Règles strictes** :
  - Déterminisme absolu des graines aléatoires (`FRandomStream` synchronisé).
  - Validation de la connexité du donjon (aucun cul-de-sac bloquant, distance minimale de 80 mètres entre spawn et extraction).
  - Instanced Static Meshes (ISM) obligatoires pour les dalles de sol, cloisons et néons.
  - Adaptation visuelle et d'ambiance aux 11 biomes (Level 0, Level 1, Level 2, Level 37 Poolrooms, etc.).

### 3. `bestiary_ai_engineer`
- **Domaine** : Intelligence Artificielle du bestiaire (9 entités hostiles), Behavior Trees, Blackboards, EQS.
- **Règles strictes** :
  - Logique asymétrique et vulnérabilité absolue du joueur : pas d'armes à feu, fuite et furtivité obligatoires.
  - Comportements canoniques respectés (ex: le Smiler attaque dès qu'il est éclairé ou fixé, le Hound traque à l'ouïe).
  - Dégâts physiques synchronisés avec les `AnimNotifies` des attaques de mêlée.

### 4. `audio_ambience_engineer`
- **Domaine** : Audio diégétique 3D spatialisé, chat vocal de proximité (VoIP), filtre talkie-walkie et paysages sonores d'angoisse.
- **Règles strictes** :
  - Atténuation réaliste avec réverbération dynamique selon la taille des pièces.
  - Occlusion acoustique à travers les cloisons (filtre passe-bas progressif).
  - Effets radio diégétiques pour l'outil Walkie-Talkie (distorsion, bruit blanc, interférences magnétiques).

### 5. `blender_3d_modeller`
- **Domaine** : Modélisation 3D via Blender 5.2.1 LTS (`F:\blender\blender.exe`), rigging d'armatures, textures PBR et pipeline d'export FBX.
- **Règles strictes** :
  - Échelle 1 unité = 1 centimètre (conforme Unreal Engine).
  - Normales orientées vers l'extérieur, UV unwrapping propre sans chevauchement.
  - Matériaux PBR standardisés : Albedo (BaseColor), Roughness, Metallic, Normal (DirectX Y-inverted si nécessaire), Emissive.

### 6. `qa_release_sentinel`
- **Domaine** : Tests d'automatisation continus, audit d'intégrité (31/31 contrôles), certification de packaging Shipping sans crash.
- **Règles strictes** :
  - Maintien obligatoire de 100% de réussite sur les 30 tests natifs (`Run_Automation_Tests.ps1`).
  - Validation des 31 contrôles dans `Run_Auto_Check.ps1`.
  - Vérification du lancement autonome sans console ni fenêtre de débogage pour `Builds/Windows/MEG_Reclamation.exe`.

---

## 2. Protocole de Collaboration Multi-Agents
1. Tout changement architectural C++ doit être soumis à validation par `ue_cpp_architect`.
2. Toute nouvelle entité ou objet 3D doit transiter par le pipeline Blender -> FBX -> Import UE5 géré par `blender_3d_modeller`.
3. Toute modification doit passer avec succès la suite `qa_release_sentinel` avant livraison.
