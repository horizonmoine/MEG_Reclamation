"""
Import SK_Hound and its animations into Unreal Engine 5.8
Destination: /Game/Characters/Bestiary/Hound/
"""

import os
import unreal

FBX_PATH = r"F:\MEG_Reclamation\RawAssets\FBX\Bestiary\SK_Hound.fbx"
DEST_PATH = "/Game/Characters/Bestiary/Hound"

def log(msg):
    print(f"[IMPORT_HOUND] {msg}")

def run_import():
    if not os.path.exists(FBX_PATH):
        log(f"ERROR: FBX not found at {FBX_PATH}")
        return False

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    # 1. Import Skeletal Mesh
    log("Importing Skeletal Mesh...")
    task = unreal.AssetImportTask()
    task.filename = FBX_PATH
    task.destination_path = DEST_PATH
    task.destination_name = "SK_Hound"
    task.replace_existing = True
    task.automated = True
    task.save = True

    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_textures = False
    options.import_materials = False
    options.import_as_skeletal = True
    options.import_animations = True
    options.create_physics_asset = True

    task.options = options
    asset_tools.import_asset_tasks([task])

    sk_mesh = unreal.load_asset(f"{DEST_PATH}/SK_Hound")
    if not sk_mesh:
        log("ERROR: Failed to load imported SK_Hound")
        return False
    log(f"SK_Hound loaded successfully: {sk_mesh}")

    # Check skeleton and physics asset
    skeleton = unreal.load_asset(f"{DEST_PATH}/SK_Hound_Skeleton")
    phys_asset = unreal.load_asset(f"{DEST_PATH}/SK_Hound_PhysicsAsset")
    log(f"Skeleton: {skeleton}")
    log(f"PhysicsAsset: {phys_asset}")

    # Check animations in folder
    anim_assets = unreal.EditorAssetLibrary.list_assets(DEST_PATH, recursive=False)
    log(f"Assets in {DEST_PATH}:")
    for a in anim_assets:
        log(f"  {a}")

    return True

if __name__ == "__main__":
    success = run_import()
    log(f"Import finished with success={success}")
