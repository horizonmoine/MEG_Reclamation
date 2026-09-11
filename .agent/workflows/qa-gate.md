---
description: Porte de qualité qa_release_sentinel avant livraison
---

1. // turbo
   `python scripts/ci/lint_ue_network.py Source`
2. Build Development Editor, Development Win64, Shipping Win64. Chaque build : `0 error(s), 0 warning(s)`.
3. `powershell -File Run_Automation_Tests.ps1` : 100 % des tests natifs.
4. `powershell -File Run_Auto_Check.ps1` : tous les contrôles verts.
5. Si la mission touche au packaging : `powershell -File Package_Shipping_Build.ps1`, lancer `Builds/Windows/MEG_Reclamation.exe` 60 s, vérifier absence de console et de crash dans `Saved/Logs`.
6. Rapport `.agents/sentinel/qa_gate_<date>.md` : résultats bruts de chaque étape, verdict final. Aucune livraison sans verdict CERTIFIÉ.
