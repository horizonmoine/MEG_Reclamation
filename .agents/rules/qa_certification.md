# Règles de Certification Qualité et Automatisation

1. Intégrité Continue :
   - `Run_Automation_Tests.ps1` doit valider les 30 tests UE 5.8 avec le code de sortie 0.
   - `Run_Auto_Check.ps1` doit valider l'ensemble des 31 contrôles d'intégrité (maps, livrables, binaries, python).

2. Packaging Shipping :
   - Le binaire `Builds/Windows/MEG_Reclamation.exe` doit être produit sans symbole de débogage et lancer le jeu directement en plein écran fenêtré sans console parasite.
   - Zéro avertissement de shader manquant ou de plugin expérimental instable.
