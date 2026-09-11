---
description: Exécute une mission C++ numérotée (01..06) depuis docs/missions avec la porte de qualité complète
---

1. Lire `docs/missions/00_PROTOCOL.md`, puis `docs/missions/<numéro>_*.md` correspondant à l'argument fourni.
2. Lire tous les fichiers listés dans la section « Contexte » de la mission. Ne pas écrire avant d'avoir lu.
3. Rédiger un plan (fichiers créés/modifiés, points de réplication, tests) et attendre validation humaine.
4. Implémenter fichier par fichier, code complet, aucun `...`.
5. // turbo
   Exécuter `python scripts/ci/lint_ue_network.py Source` et corriger jusqu'à 0 erreur.
6. Exécuter `F:\UE_5.8\Engine\Build\BatchFiles\Build.bat MEG_ReclamationEditor Win64 Development -Project="F:\MEG_Reclamation\MEG_Reclamation.uproject" -WaitMutex` et corriger jusqu'à `0 error(s), 0 warning(s)`.
7. Exécuter `powershell -File Run_Automation_Tests.ps1` puis `powershell -File Run_Auto_Check.ps1`. Tout doit être vert.
8. Lancer le workflow `/architect-review`, puis `/qa-gate`.
9. Créer la branche `feature/mission-<numéro>-<slug>` et un commit `feat(<domaine>): Mission <numéro> - <titre>` avec le rapport de la porte de qualité.
