import unreal

assets = [
    ("/Game/Meshes/Modular/SM_Floor_Tile_400x400", "/Game/Meshes/Modular/Mat_Floor_Carpet"),
    ("/Game/Meshes/Modular/SM_Wall_Modular_400x300", "/Game/Meshes/Modular/Mat_Wall_Wallpaper"),
    ("/Game/Meshes/Modular/SM_Ceiling_Tile_400x400", "/Game/Meshes/Modular/Mat_Ceiling_Tile"),
    ("/Game/Meshes/Modular/SM_Pillar_Industrial", "/Game/Meshes/Modular/Mat_Pillar_Concrete"),
    ("/Game/Meshes/Modular/SM_Ceiling_FluorescentLight", "/Game/Meshes/Modular/Mat_Light_MetalHousing"),
    ("/Game/Meshes/Props/SM_Office_Desk", "/Game/Meshes/Props/Mat_Desk_FauxWoodLaminate"),
    ("/Game/Meshes/Props/SM_Office_Chair", "/Game/Meshes/Props/Mat_Chair_RoughBrownFabric"),
    ("/Game/Meshes/Props/SM_Terminal_MEG", "/Game/Meshes/Props/Mat_Terminal_SteelCase"),
    ("/Game/Meshes/Props/SM_HidingLocker", "/Game/Meshes/Props/Mat_Locker_OliveGreen"),
    ("/Game/Meshes/Props/SM_SupplyCrate_MEG", "/Game/Meshes/Props/Mat_MEGCrate_OlivePolymer"),
    ("/Game/Meshes/Props/SM_Exit_Sign", "/Game/Meshes/Props/Mat_ExitSign_Housing"),
    ("/Game/Meshes/Props/SM_Vending_Machine", "/Game/Meshes/Props/Mat_Locker_OliveGreen"),
    ("/Game/Meshes/Props/SM_Loot_Cart", "/Game/Meshes/Props/Mat_Crate_BlackTrim"),
]

print("=== CHECKING STATIC MESH MATERIALS ===")
for mesh_path, mat_path in assets:
    mesh = unreal.load_asset(mesh_path)
    if not mesh:
        print(f"[MISSING MESH] {mesh_path}")
        continue
    
    num_mats = mesh.get_num_sections(0) if hasattr(mesh, 'get_num_sections') else 1
    mats = mesh.get_editor_property('static_materials')
    print(f"Mesh: {mesh_path.split('/')[-1]} - Materials count: {len(mats)}")
    for i, sm in enumerate(mats):
        mat_interface = sm.get_editor_property('material_interface')
        mat_name = mat_interface.get_name() if mat_interface else "None (DEFAULT CHECKERBOARD!)"
        print(f"  Slot {i} [{sm.get_editor_property('material_slot_name')}]: {mat_name}")
        if not mat_interface:
            target_mat = unreal.load_asset(mat_path)
            if target_mat:
                mesh.set_material(i, target_mat)
                print(f"    -> ASSIGNED {target_mat.get_name()} to slot {i}")
                unreal.EditorAssetLibrary.save_loaded_asset(mesh)
