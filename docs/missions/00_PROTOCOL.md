# Protocole d'exécution des missions IA

## Ordre recommandé
`07 -> 01 -> 02 -> 03 -> 04 -> 08 -> 09 -> 05` puis `06` (optionnelle).
- `07` (Hub et règles de zone) est indépendante et corrige un bug visible : la lancer en premier.
- `01` à `04` : fondations réseau, séquentielles (le PlayerState est la base).
- `08` (procgen) et `09` (directeur) reposent sur `02`, `04`, `07`.
- `05` (persistance, Shipping) ferme la boucle.
Références design : `docs/design/LEVEL_DESIGN_BIBLE.md`. Architecture : `docs/architecture/NETWORK_ARCHITECTURE.md`.

## Lancement
- **Gemini CLI** : `gemini` à la racine, puis coller le contenu de `docs/missions/0X_*.md`. GEMINI.md et `.agents/rules/ue5_cpp_network_rules.md` sont chargés automatiquement.
- **OpenCode** : `/mission 01` (commande `.opencode/command/mission.md`) avec l'agent `ue-cpp-architect`.
- **Antigravity** : workflow `/mission 01` (`.agent/workflows/mission.md`).

## Porte de qualité (obligatoire avant tout commit)
1. `python scripts/ci/lint_ue_network.py Source` -> 0 erreur.
2. Build Development Editor : `0 error(s), 0 warning(s)`.
3. `Run_Automation_Tests.ps1` -> 100 %.
4. `Run_Auto_Check.ps1` -> tous les contrôles verts.
5. Revue par l'agent `architect-review` (`.agent/workflows/architect-review.md` ou `/architect-review` OpenCode).
6. Revue par `qa-gate`.

## Signaux de rejet immédiat
- Un `float` répliqué nommé `*TimeRemaining` ou décrémenté au tick.
- Un `UFUNCTION(Server, ...)` sur `ALiminalGameState` ou `ALiminalGameMode`.
- Un pointeur brut `UPROPERTY` (`AFoo* Bar;`) au lieu de `TObjectPtr`.
- Un `TArray<UObject*>` ou `TMap` répliqué.
- Des `...` ou du pseudo-code dans un fichier livré.
- Un fichier modifié hors du périmètre listé dans la mission.

En cas de rejet : relancer la mission en citant explicitement la section 4 de `.agents/rules/ue5_cpp_network_rules.md`.

## Commit
Un commit par mission, message : `feat(<domaine>): Mission 0X - <titre>`. Branche `feature/mission-0X-<slug>`. MR vers `main` avec le rapport de la porte de qualité collé en description.
