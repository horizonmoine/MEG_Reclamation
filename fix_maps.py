import unreal

print("=== INSPECTING MAPS ===")
maps_to_fix = [
    ("/Game/Maps/Lvl_MainMenu", "/Script/MEG_Reclamation.LiminalMainMenuGameMode", True),
    ("/Game/Maps/Lvl_Hub_BaseAlpha", "/Script/MEG_Reclamation.LiminalLobbyGameMode", False),
]

for map_path, expected_gamemode_path, remove_generators in maps_to_fix:
    print(f"\n--- Processing {map_path} ---")
    world = unreal.EditorLoadingAndSavingUtils.load_map(map_path)
    if not world:
        print(f"[ERROR] Failed to load {map_path}")
        continue
    
    # Get World Settings
    editor_world = unreal.EditorLevelLibrary.get_editor_world()
    world_settings = editor_world.get_world_settings()
    current_gm = world_settings.get_editor_property("default_game_mode")
    print(f"Current default_game_mode: {current_gm}")

    # Set new GameMode
    target_gm_class = unreal.load_class(None, expected_gamemode_path)
    print(f"Target GM class: {target_gm_class}")
    if target_gm_class:
        world_settings.set_editor_property("default_game_mode", target_gm_class)
        print(f"[OK] Set default_game_mode to {target_gm_class.get_name()}")

    # If remove_generators is True, clean up any LiminalLevelGenerator in this map
    if remove_generators:
        actors = unreal.EditorLevelLibrary.get_all_level_actors()
        for a in actors:
            cname = a.get_class().get_name()
            if "LevelGenerator" in cname or "LiminalLevelGenerator" in cname:
                print(f"Removing generator actor: {a.get_name()}")
                unreal.EditorLevelLibrary.destroy_actor(a)

    # Save level
    saved = unreal.EditorLoadingAndSavingUtils.save_current_level()
    print(f"Save status for {map_path}: {saved}")

print("\n=== MAP FIX COMPLETE ===")
