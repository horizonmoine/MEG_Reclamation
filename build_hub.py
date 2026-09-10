import unreal
import sys

def build_hub():
    map_path = "/Game/Maps/Lvl_Hub_BaseAlpha"
    print(f"Loading {map_path}...")
    world = unreal.EditorLoadingAndSavingUtils.load_map(map_path)
    if not world:
        print(f"[ERROR] Failed to load {map_path}")
        return

    # Clean up existing LevelGenerator
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    for a in actors:
        cname = a.get_class().get_name()
        if "LevelGenerator" in cname or "LiminalLevelGenerator" in cname:
            unreal.EditorLevelLibrary.destroy_actor(a)
        if "TerminalActor" in cname or "LiminalTerminalActor" in cname:
            unreal.EditorLevelLibrary.destroy_actor(a)
        if "StaticMeshActor" in cname:
            unreal.EditorLevelLibrary.destroy_actor(a)

    print("Cleaned up old actors.")

    # Get PlayerStart location
    player_loc = unreal.Vector(0, 0, 110)
    for a in actors:
        if a.get_class().get_name() == "PlayerStart":
            player_loc = a.get_actor_location()
            break

    # Spawn Terminal Actor in front of PlayerStart
    terminal_class = unreal.load_class(None, "/Script/MEG_Reclamation.LiminalTerminalActor")
    if terminal_class:
        term_loc = player_loc + unreal.Vector(300, 0, -110) # 3m ahead, lower Z
        term_rot = unreal.Rotator(0, 180, 0)
        unreal.EditorLevelLibrary.spawn_actor_from_class(terminal_class, term_loc, term_rot)
        print("Spawned LiminalTerminalActor.")
    else:
        print("[ERROR] Could not load TerminalActor class.")

    # Spawn walls/floor/ceiling to make a 4x4 room
    wall_mesh = unreal.EditorAssetLibrary.load_asset("/Game/Meshes/Environment/SM_Office_Wall")
    floor_mesh = unreal.EditorAssetLibrary.load_asset("/Game/Meshes/Environment/SM_Office_Floor")
    ceil_mesh = unreal.EditorAssetLibrary.load_asset("/Game/Meshes/Environment/SM_Office_Ceiling")
    
    if wall_mesh and floor_mesh:
        room_size = 4
        tile_size = 400.0 # typically 400x400
        start_x = player_loc.x - (room_size/2)*tile_size
        start_y = player_loc.y - (room_size/2)*tile_size
        z_floor = player_loc.z - 110.0
        z_ceil = z_floor + 400.0
        
        for x in range(room_size):
            for y in range(room_size):
                fx = start_x + x * tile_size
                fy = start_y + y * tile_size
                
                # Floor
                f_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(fx, fy, z_floor), unreal.Rotator(0,0,0))
                f_actor.static_mesh_component.set_static_mesh(floor_mesh)
                
                # Ceiling
                c_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(fx, fy, z_ceil), unreal.Rotator(180,0,0))
                if ceil_mesh:
                    c_actor.static_mesh_component.set_static_mesh(ceil_mesh)
                else:
                    c_actor.static_mesh_component.set_static_mesh(floor_mesh) # fallback

                # Walls
                if x == 0:
                    w_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(fx - tile_size/2, fy, z_floor), unreal.Rotator(0, 0, 0))
                    w_actor.static_mesh_component.set_static_mesh(wall_mesh)
                if x == room_size - 1:
                    w_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(fx + tile_size/2, fy, z_floor), unreal.Rotator(0, 180, 0))
                    w_actor.static_mesh_component.set_static_mesh(wall_mesh)
                if y == 0:
                    w_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(fx, fy - tile_size/2, z_floor), unreal.Rotator(0, 90, 0))
                    w_actor.static_mesh_component.set_static_mesh(wall_mesh)
                if y == room_size - 1:
                    w_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(fx, fy + tile_size/2, z_floor), unreal.Rotator(0, -90, 0))
                    w_actor.static_mesh_component.set_static_mesh(wall_mesh)

        print("Spawned room meshes.")
    else:
        print("[ERROR] Could not load meshes for the room.")

    unreal.EditorLoadingAndSavingUtils.save_current_level()
    print("Hub map saved successfully.")

build_hub()
