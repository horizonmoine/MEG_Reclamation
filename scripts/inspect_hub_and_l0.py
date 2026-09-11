import unreal
import json

def inspect_map(map_name):
    editor_sub = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    asset_path = f"/Game/Maps/{map_name}"
    print(f"\n==================================================")
    print(f"=== INSPECTION MAP: {map_name} ({asset_path}) ===")
    print(f"==================================================")
    try:
        loaded = editor_sub.load_level(asset_path)
        print(f"Load result: {loaded}")
    except Exception as e:
        print(f"Exception loading map: {e}")
        return None

    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    if not world:
        print("Error: No editor world available.")
        return None

    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    print(f"Total Actors: {len(actors)}")

    data = {
        "map": map_name,
        "total_actors": len(actors),
        "actors": {}
    }

    critical_actors = {}

    for a in actors:
        cls_name = a.get_class().get_name()
        lbl = a.get_actor_label()
        loc = a.get_actor_location()
        data["actors"][cls_name] = data["actors"].get(cls_name, 0) + 1

        if any(keyword in cls_name.lower() for keyword in ["terminal", "airlock", "generator", "extraction", "breaker", "fuse", "smiler", "hound", "clump", "playerstart"]):
            critical_actors[lbl] = {
                "class": cls_name,
                "location": [loc.x, loc.y, loc.z]
            }

    print("Actor Classes Breakdown:")
    for cls_name, count in sorted(data["actors"].items()):
        print(f"  - {cls_name}: {count}")

    print("\nCritical Gameplay Actors:")
    for lbl, info in sorted(critical_actors.items()):
        print(f"  * {lbl} [{info['class']}] at {info['location']}")

    return data, critical_actors

if __name__ == "__main__":
    hub_data = inspect_map("Lvl_Hub_BaseAlpha")
    l0_data = inspect_map("Lvl_00_Lobby")
