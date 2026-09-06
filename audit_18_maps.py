import unreal

editor_sub = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

maps = [
    "Lvl_MainMenu",
    "Lvl_Hub_BaseAlpha",
    "Lvl_00_Lobby",
    "Lvl_01_HabitableZone",
    "Lvl_02_PipeDreams",
    "Lvl_03_ElectricalStation",
    "Lvl_04_AbandonedOffice",
    "Lvl_06_LightsOut",
    "Lvl_08_CaveSystem",
    "Lvl_09_DarkSuburbs",
    "Lvl_10_WheatFields",
    "Lvl_37_Poolrooms",
    "Lvl_99_RunForYourLife",
    "Lvl_Loop",
    "Lvl_ProcGen",
    "Lvl_Level0_Massive",
    "Lvl_Level37_Poolrooms",
    "Lvl_LevelRun_Gauntlet"
]

print("==================================================")
print("=== AUDIT LEVEL DESIGN DES 18 MAPS MEG RECLAMATION ===")
print("==================================================")

for map_name in maps:
    asset_path = f"/Game/Maps/{map_name}"
    print(f"\n>>> MAP: {map_name} ({asset_path})")
    try:
        ok = editor_sub.load_level(asset_path)
    except Exception as e:
        print(f"  [ERROR] Cannot load level: {e}")
        continue

    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    if not world:
        print("  [ERROR] No world")
        continue

    ws = world.get_world_settings()
    gm = ws.get_editor_property('default_game_mode') if ws else None
    gm_name = gm.get_name() if gm else "None"
    print(f"  WorldSettings GameMode: {gm_name}")

    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    print(f"  Total Actors Count: {len(all_actors)}")

    actor_classes = {}
    for a in all_actors:
        cls_name = a.get_class().get_name()
        actor_classes[cls_name] = actor_classes.get(cls_name, 0) + 1

        if "LiminalLevelGenerator" in cls_name:
            try:
                biome = a.get_editor_property("biome")
                scale = a.get_editor_property("map_scale")
                seed = a.get_editor_property("seed")
                print(f"  [GENERATOR] Biome={biome} Scale={scale} Seed={seed}")
            except Exception as ex:
                print(f"  [GENERATOR] Prop read err: {ex}")

        if "PostProcessVolume" in cls_name:
            try:
                unbound = a.get_editor_property("unbound")
                print(f"  [POST_PROCESS] Label={a.get_actor_label()} Unbound={unbound}")
            except Exception as ex:
                print(f"  [POST_PROCESS] {ex}")

        if "AudioVolume" in cls_name:
            print(f"  [AUDIO_VOLUME] Label={a.get_actor_label()}")

        if "Airlock" in cls_name:
            print(f"  [AIRLOCK] Label={a.get_actor_label()}")

        if "Terminal" in cls_name:
            print(f"  [TERMINAL] Label={a.get_actor_label()}")

        if "ExtractionZone" in cls_name:
            print(f"  [EXTRACTION] Label={a.get_actor_label()}")

    print("  Actor breakdown:")
    for k, v in sorted(actor_classes.items()):
        print(f"    - {k}: {v}")

print("\n==================================================")
print("=== AUDIT COMPLETE ===")
print("==================================================")
