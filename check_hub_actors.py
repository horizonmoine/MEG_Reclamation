import unreal

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/Lvl_Hub_BaseAlpha")
actors = unreal.EditorLevelLibrary.get_all_level_actors()
for a in actors:
    loc = a.get_actor_location()
    rot = a.get_actor_rotation()
    print(f"Actor: {a.get_name():30} Class: {a.get_class().get_name():25} Loc: ({loc.x:8.1f}, {loc.y:8.1f}, {loc.z:8.1f})")
