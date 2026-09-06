# Original User Request

## 2026-09-05T23:45:11Z

Finalisation et polish commercial AAA indie de **M.E.G. : Reclamation** (Unreal Engine 5.8), un jeu d'extraction coopératif d'horreur procédurale inspiré de *Escape the Backrooms* et *Lethal Company*. Le projet dispose déjà d'un moteur C++ solide, de 18 maps, de 42 textures PBR, de 13 meshes 3D avec audio physique, de 9 IA sous Behavior Trees et d'une suite de 30 tests automatisés 100% validée.

Working directory: F:/MEG_Reclamation
Integrity mode: development

## Requirements

### R1. Systèmes d'Animation Squelettique & Bestiaire Animé
- Intégrer les squelettes et Animation Blueprints pour le Scavenger (bras vue subjective, corps complet troisième personne pour le multijoueur).
- Remplacer les poses statiques des 9 entités hostiles (Hound, Smiler, Partygoer, Clump, Deathmoth, Duller, Jerry, Skinwalker, Wretch) par des maillages squelettiques animés avec cycles de marche/course (locomotion blendspaces), états d'agression, d'attaque physique et de mort.
- Connecter les événements d'animation (AnimNotifies) aux sons d'attaque et aux traces de dégâts C++.

### R2. Chat Vocal de Proximité Spatialisé & Radio Diégétique
- Activer et peaufiner le composant C++ de voix de proximité (`LiminalProximityVoiceComponent`).
- Appliquer une atténuation spatiale 3D avec réverbération dynamique selon le volume des salles et étouffement sonore à travers les cloisons (occlusion acoustique).
- Intégrer le filtre audio passe-bande radio/talkie-walkie avec bruits statiques et distorsions électromagnétiques lorsque le joueur utilise l'outil `WalkieTalkieTool`.

### R3. Boucle d'Extraction, Sas M.E.G. & Économie de Quota
- Parfaire la cinématique et le sas de décompression d'incursion (`LiminalAirlockActor`) reliant la Base Alpha aux niveaux des Backrooms.
- Implémenter le système de livraison physique des commandes du terminal M.E.G. (capsule pneumatique ou monte-charge diégétique) lors de l'achat d'outils et consommables dans le store.
- Peaufiner l'écran de débriefing de fin d'expédition (`LiminalDebriefHUD`) avec récapitulatif des objets ramenés, calcul du quota de la corporation M.E.G. et sanctions en cas d'échec.

### R4. Intégration Steam / Matchmaking en Ligne
- Configurer les sous-systèmes en ligne (`OnlineSubsystemSteam` / `OnlineSubsystemNull`) pour le matchmaking P2P, l'hébergement de sessions publiques/privées et les invitations d'amis.
- Mettre en place les succès Steam diégétiques (exploration des biomes, survie face aux entités, atteinte des quotas d'extraction).

### R5. Polish Visuel & Scalabilité Graphique
- Optimiser les profils de scalabilité pour garantir 60+ FPS stables sur configurations moyennes (GeForce GTX 1060 / RTX 2060).
- Finaliser la navigation complète à la manette (gamepad) sur tous les menus (HUD, Main Menu, Pause, Terminal de vente, Inventaire Tetris).

## Acceptance Criteria

### Compilation & Tests
- [ ] Le projet compile sans erreur ni avertissement en `Development Win64` et en `Shipping Win64`.
- [ ] La suite de 30 tests d'automatisation (`Run_Automation_Tests.ps1`) s'exécute avec 100% de réussite (Exit Code 0).
- [ ] Le script d'audit d'intégrité (`Run_Auto_Check.ps1`) valide 27/27 contrôles.

### Gameplay & Immersion
- [ ] Les 9 monstres disposent d'animations de traque fluides et infligent des dégâts synchronisés avec leurs mouvements.
- [ ] Le chat vocal de proximité fonctionne de manière directionnelle entre plusieurs joueurs en réseau local ou Internet.
- [ ] Le cycle complet de boucle de gameplay (Base Alpha -> Incursion sas -> Récolte de scrap -> Fuite sas d'extraction -> Débriefing -> Paiement du quota) s'enchaîne de façon fluide sans crash ni déconnexion.
- [ ] L'exécutable Standalone Shipping `Builds/Windows/MEG_Reclamation.exe` démarre instantanément, sans console ni erreur de shaders.

## Follow-up — 2026-09-05T23:46:50Z

Directive fondamentale de Game Design et de Cohérence Globale (Directives Utilisateur) :

Ce projet n'est pas une simple collection de classes techniques ou de boîtes de collision, c'est un JEU COMPLET d'horreur coopérative (Escape the Backrooms x Lethal Company). Chaque système doit servir l'expérience joueur, le game feel, l'ambiance et la cohérence diégétique :

1. Menus & UX/UI :
   - Menus entièrement thématisés rétro-analogique / CRT / terminal M.E.G. (bruits de touches mécaniques, bip de confirmation, grésillement de scanlines).
   - HUD diégétique immersif : camescope VHS avec timestamp, batterie résiduelle de la lampe, jauge d'endurance réactive au sprint, réticule dynamique affichant les détails précis de l'objet ciblé ("E - Ramasser [Bobine de cuivre] | 5 kg | 45 Crédits").
   - Menus Pause et Débriefing scénarisés : rapport officiel de la corporation M.E.G. évaluant la performance de l'équipe, sanctions en cas de quota non atteint.

2. Gameplay & Game Loop Cohérente :
   - Base Alpha (Hub) : ambiance industrielle vivante, tableau des quotas, terminal de commande avec livraison physique.
   - Incursion sas : sirène d'alerte, décompression, passage sans temps de chargement vers le dédale liminal.
   - Tension & Survie : gestion de l'endurance, bruits de pas spatialisés alertant les créatures selon le sol, hallucinations de folie dans l'obscurité.
   - Bestiaire avec contre-mesures identifiables : chaque créature doit imposer une tactique de fuite ou de dissimulation spécifique.

Assurez-vous que chaque livrable (R1 à R5) s'intègre harmonieusement dans cette boucle de jeu globale.

