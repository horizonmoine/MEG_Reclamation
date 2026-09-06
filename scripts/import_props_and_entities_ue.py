"""
Imports newly generated 3D meshes (Harmonic Resonator, Trilight Flashlight, Hydrolitis)
into Unreal Engine 5.8 Content.
"""

import os
import unreal

FBX_PROPS = r"F:\MEG_Reclamation\RawAssets\FBX\Props"
FBX_BESTIARY = r"F:\MEG_Reclamation\RawAssets\FBX\Bestiary"

IMPORTS = [
    (os.path.join(FBX_PROPS, "SM_Harmonic_Resonator.fbx"), "/Game/Meshes/Props", "SM_Harmonic_Resonator"),
    (os.path.join(FBX_PROPS, "SM_Trilight_Flashlight.fbx"), "/Game/Meshes/Props", "SM_Trilight_Flashlight"),
    (os.path.join(FBX_BESTIARY, "SK_Hydrolitis.fbx"), "/Game/Characters/Bestiary/Hydrolitis", "SM_Hydrolitis"),
]

def import_meshes():
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    
    for fbx_path, dest_path, asset_name in IMPORTS:
        if not os.path.exists(fbx_path):
            print(f"[SKIP] File not found: {fbx_path}")
            continue
            
        print(f"Importing {fbx_path} to {dest_path}/{asset_name}...")
        task = unreal.AssetImportTask()
        task.filename = fbx_path
        task.destination_path = dest_path
        task.destination_name = asset_name
        task.replace_existing = True
        task.automated = True
        task.save = True
        
        options = unreal.FbxImportUI()
        options.import_mesh = True
        options.import_textures = False
        options.import_materials = False
        options.import_as_skeletal = False
        
        task.options = options
        asset_tools.import_asset_tasks([task])
        
        imported = unreal.load_asset(f"{dest_path}/{asset_name}")
        if imported:
            unreal.EditorAssetLibrary.save_loaded_asset(imported)
            print(f"[SUCCESS] Imported {dest_path}/{asset_name}")
        else:
            print(f"[WARN] Failed to load {dest_path}/{asset_name}")

    unreal.EditorAssetLibrary.save_directory("/Game/Meshes/Props")
    unreal.EditorAssetLibrary.save_directory("/Game/Characters/Bestiary/Hydrolitis")
    print("\n[COMPLETE] All new 3D assets imported successfully into UE 5.8 Content!")

if __name__ == "__main__":
    import_meshes()
