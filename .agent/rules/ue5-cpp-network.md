---
trigger: always_on
---

Applique intégralement `.agents/rules/ue5_cpp_network_rules.md` et `docs/architecture/NETWORK_ARCHITECTURE.md` pour tout fichier sous `Source/`.
Modèle 100 % Server-Authoritative : le serveur décide, le client demande et affiche.
Rejette immédiatement : float répliqué `*TimeRemaining`, Server RPC sur GameState/GameMode, pointeur brut UPROPERTY, `TArray<UObject*>` ou `TMap` répliqués, pseudo-code ou `...`.
Avant tout commit : `python scripts/ci/lint_ue_network.py Source`, build 0 warning, `Run_Automation_Tests.ps1` 100 %, `Run_Auto_Check.ps1` vert.
