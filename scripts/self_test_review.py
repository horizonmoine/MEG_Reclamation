import unreal
import json
import os

log = unreal.log
log_warning = unreal.log_warning
log_error = unreal.log_error

results = {
    "title": "M.E.G. : RECLAMATION -- TEST ET RAPPORT CRITIQUE COMPLET",
    "maps_tested": {},
    "systems_evaluated": {}
}

les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)

log("===================================================================")
log("  LANCEMENT DU TEST AUTONOME APPROFONDI : M.E.G. RECLAMATION")
log("===================================================================")

# 1. TEST DE LA CARTE MENU PRINCIPAL (Lvl_MainMenu)
log("\n[TEST 1/3] Chargement et diagnostic de Lvl_MainMenu...")
try:
    les.load_level("/Game/Maps/Lvl_MainMenu")
    world = ues.get_editor_world()
    ws = world.get_world_settings() if world else None
    gm = ws.get_editor_property('default_game_mode') if ws else None
    gm_name = gm.get_name() if gm else "None"
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    results["maps_tested"]["Lvl_MainMenu"] = {
        "status": "PASS" if gm_name == "LiminalMainMenuGameMode" else "WARN",
        "default_game_mode": gm_name,
        "actor_count": len(actors)
    }
    log(f"  -> Lvl_MainMenu OK: GameMode={gm_name}, Acteurs={len(actors)}")
except Exception as e:
    log_error(f"  -> Erreur Lvl_MainMenu: {e}")
    results["maps_tested"]["Lvl_MainMenu"] = {"status": "FAIL", "error": str(e)}

# 2. TEST DE LA CARTE DU HUB BASE ALPHA (Lvl_Hub_BaseAlpha)
log("\n[TEST 2/3] Chargement et diagnostic de Lvl_Hub_BaseAlpha...")
try:
    les.load_level("/Game/Maps/Lvl_Hub_BaseAlpha")
    world = ues.get_editor_world()
    ws = world.get_world_settings() if world else None
    gm = ws.get_editor_property('default_game_mode') if ws else None
    gm_name = gm.get_name() if gm else "None"
    
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    actor_names = [a.get_name() for a in actors]
    actor_classes = [a.get_class().get_name() for a in actors]
    
    has_terminal = any("Terminal" in c for c in actor_classes)
    has_airlock = any("Airlock" in c for c in actor_classes)
    has_player_start = any("PlayerStart" in c for c in actor_classes)
    has_extraction = any("Extraction" in c for c in actor_classes)
    
    monsters_found = [n for n, c in zip(actor_names, actor_classes) if any(m in c.lower() for m in ["hound", "smiler", "partygoer", "skinstealer", "clump", "duller", "jerry", "hydrolitis", "wretch"])]
    
    results["maps_tested"]["Lvl_Hub_BaseAlpha"] = {
        "status": "PASS" if (has_terminal and has_airlock and has_player_start and len(monsters_found) == 0) else "FAIL",
        "default_game_mode": gm_name,
        "actor_count": len(actors),
        "has_terminal_pc": has_terminal,
        "has_airlock_sas": has_airlock,
        "has_player_start": has_player_start,
        "has_extraction_depot": has_extraction,
        "hostiles_in_hub": monsters_found,
        "is_safe_zone": len(monsters_found) == 0
    }
    log(f"  -> Lvl_Hub_BaseAlpha OK: GameMode={gm_name}, Terminal={has_terminal}, Sas={has_airlock}, Monstre(s)={len(monsters_found)}")
except Exception as e:
    log_error(f"  -> Erreur Lvl_Hub_BaseAlpha: {e}")
    results["maps_tested"]["Lvl_Hub_BaseAlpha"] = {"status": "FAIL", "error": str(e)}

# 3. TEST DE LA CARTE DE MISSION (Lvl_00_Lobby)
log("\n[TEST 3/3] Chargement et diagnostic de Lvl_00_Lobby...")
try:
    les.load_level("/Game/Maps/Lvl_00_Lobby")
    world = ues.get_editor_world()
    ws = world.get_world_settings() if world else None
    gm = ws.get_editor_property('default_game_mode') if ws else None
    gm_name = gm.get_name() if gm else "None"
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    results["maps_tested"]["Lvl_00_Lobby"] = {
        "status": "PASS",
        "default_game_mode": gm_name,
        "actor_count": len(actors)
    }
    log(f"  -> Lvl_00_Lobby OK: Acteurs={len(actors)}")
except Exception as e:
    log_error(f"  -> Erreur Lvl_00_Lobby: {e}")
    results["maps_tested"]["Lvl_00_Lobby"] = {"status": "FAIL", "error": str(e)}

# 4. EVALUATION DES SYSTEMES
log("\n[SYSTEMES] Verification des assets de donnees...")
dt_loot = unreal.load_object(None, "/Game/Data/DT_LootItems.DT_LootItems")
loot_rows = dt_loot.get_row_names() if dt_loot else []
results["systems_evaluated"]["DT_LootItems"] = {
    "loaded": dt_loot is not None,
    "row_count": len(loot_rows),
    "items": [str(r) for r in loot_rows]
}

imc = unreal.load_object(None, "/Game/Input/Scavenger/IMC_Scavenger.IMC_Scavenger")
results["systems_evaluated"]["IMC_Scavenger"] = {
    "loaded": imc is not None
}

output_json = "F:/MEG_Reclamation/Saved/gameplay_test_report.json"
os.makedirs(os.path.dirname(output_json), exist_ok=True)
with open(output_json, "w", encoding="utf-8") as f:
    json.dump(results, f, indent=2, ensure_ascii=False)

log(f"\n[SUCCES] Rapport de test sauvegarde dans : {output_json}")
log("===================================================================")
