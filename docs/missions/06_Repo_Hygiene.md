# MISSION 06 (optionnelle) — Hygiène du dépôt

Sans impact gameplay. À exécuter par un agent à faible coût. Aucune modification dans `Source/`.

## Outputs attendus
1. Déplacer les scripts Python racine (`audit_*.py`, `fix_*.py`, `generate_*.py`, `import_*.py`, `check_*.py`, `patch_*.py`, `populate_dt_loot.py`, `wire_audio.py`, `list_meshes.py`, `inject_bt_assign.py`, `run_batch_import.py`, `build_hub.py`, `create_ai_assets.py`) vers `scripts/pipeline/`. Mettre à jour tout chemin référencé dans `Launch_MultiAgent_Orchestrator.bat`, `orchestrate_meg_team.py`, `Run_Auto_Check.ps1`.
2. Supprimer les artefacts de sortie versionnés : `fix_audio_out*.txt`, `audit_audio_out.json`, `parse*.ps1` si non référencés (vérifier par grep). Ajouter `*_out.txt`, `*_out.json` au `.gitignore`.
3. Fusionner `MEG_ReclamationGameMode.h/.cpp` (template Epic) dans `ALiminalGameMode` si aucune map ne l'utilise (`grep -r MEG_ReclamationGameMode Config/ Content/`), sinon documenter la raison dans `docs/architecture/NETWORK_ARCHITECTURE.md`.
4. `README.md` : ajouter une section « Travailler avec les agents IA » pointant vers `docs/missions/00_PROTOCOL.md`.

## Assertions
- `Run_Auto_Check.ps1` et `Run_Automation_Tests.ps1` inchangés en résultat.
- Build Editor : 0 error, 0 warning.
