"""
Imports generated Bestiary FBX models into Unreal Engine 5.8 Content/Characters/Bestiary/
and wires BP_Hound to use the new skeletal mesh.
"""

import os
import unreal

FBX_DIR = r"F:\MEG_Reclamation\RawAssets\FBX\Bestiary"
ENTITIES = [
    ("SK_Hound.fbx", "/Game/Characters/Bestiary/Hound", "SK_Hound"),
    ("SK_Deathmoth.fbx", "/Game/Characters/Bestiary/Deathmoth", "SK_Deathmoth"),
    ("SK_Clump.fbx", "/Game/Characters/Bestiary/Clump", "SK_Clump"),
    ("SK_Jerry.fbx", "/Game/Characters/Bestiary/Jerry", "SK_Jerry"),
]

def import_bestiary():
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    
    for fbx_name, dest_path, asset_name in ENTITIES:
        fbx_path = os.path.join(FBX_DIR, fbx_name)
        if not os.path.exists(fbx_path):
            print(f"[ERROR] Missing FBX file: {fbx_path}")
            continue
            
        print(f"Importing {fbx_name} into {dest_path}...")
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
        options.import_as_skeletal = True
        options.mesh_type_to_import = unreal.FBXMeshType.FBXMT_SKELETAL_MESH
        options.import_animations = True
        
        # Skeletal mesh import data
        options.skeletal_mesh_import_data.import_morph_targets = True
        options.skeletal_mesh_import_data.use_t0_as_ref_pose = True
        task.options = options
        
        asset_tools.import_asset_tasks([task])
        
        imported = unreal.load_asset(f"{dest_path}/{asset_name}")
        if imported:
            unreal.EditorAssetLibrary.save_loaded_asset(imported)
            print(f"[SUCCESS] Imported {dest_path}/{asset_name}")
        else:
            print(f"[WARNING] Could not load imported {dest_path}/{asset_name}")

    # Wire BP_Hound to use SK_Hound
    bp_hound = unreal.load_object(None, "/Game/AI/BP_Hound.BP_Hound")
    if bp_hound:
        gen_class = bp_hound.generated_class()
        cdo = unreal.get_default_object(gen_class)
        sk_hound = unreal.load_asset("/Game/Characters/Bestiary/Hound/SK_Hound")
        if sk_hound and hasattr(cdo, "DefaultSkeletalMesh"):
            cdo.set_editor_property("DefaultSkeletalMesh", sk_hound)
            unreal.BlueprintEditorLibrary.compile_blueprint(bp_hound)
            unreal.EditorAssetLibrary.save_asset("/Game/AI/BP_Hound", only_if_is_dirty=False)
            print("[SUCCESS] BP_Hound wired to SK_Hound")

    unreal.EditorAssetLibrary.save_directory("/Game/Characters/Bestiary")
    print("Bestiary import complete!")

if __name__ == "__main__":
    import_bestiary()
