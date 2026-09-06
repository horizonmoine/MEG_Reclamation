import unreal
import os

print("==================================================")
print("=== MEG RECLAMATION: BATCH ASSET & TEXTURE IMPORT ===")
print("==================================================")

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

# 1. IMPORT FBX MESHES
SRC_BASE = r'F:\MEG_Reclamation\RawAssets\FBX'
MAPPINGS = [
    (os.path.join(SRC_BASE, 'Modular'), '/Game/Meshes/Modular'),
    (os.path.join(SRC_BASE, 'Props'), '/Game/Meshes/Props'),
    (os.path.join(SRC_BASE, 'Puzzles'), '/Game/Meshes/Puzzles'),
    (os.path.join(SRC_BASE, 'Tools'), '/Game/Meshes/Tools'),
]

mesh_count = 0
for src_dir, dest_path in MAPPINGS:
    if not os.path.exists(src_dir):
        continue
    for filename in sorted(os.listdir(src_dir)):
        if not filename.lower().endswith('.fbx'):
            continue
        file_path = os.path.join(src_dir, filename)
        asset_name = os.path.splitext(filename)[0]
        
        task = unreal.AssetImportTask()
        task.filename = file_path
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
        options.static_mesh_import_data.combine_meshes = True
        options.static_mesh_import_data.auto_generate_collision = True
        options.static_mesh_import_data.generate_lightmap_u_vs = True
        task.options = options
        
        asset_tools.import_asset_tasks([task])
        loaded = unreal.load_asset(f'{dest_path}/{asset_name}')
        if loaded:
            unreal.EditorAssetLibrary.save_loaded_asset(loaded)
            mesh_count += 1
            print(f"[MESH OK] {dest_path}/{asset_name}")
        else:
            print(f"[MESH FAIL] {dest_path}/{asset_name}")

print(f"Total Static Meshes Processed: {mesh_count}")

# 2. IMPORT PBR TEXTURES
SRC_TEX_DIR = r"F:\MEG_Reclamation\Content\Textures\Backrooms"
DEST_TEX_PATH = "/Game/Textures/Backrooms"

tex_count = 0
for filename in sorted(os.listdir(SRC_TEX_DIR)):
    if not filename.lower().endswith('.png'):
        continue
    file_path = os.path.join(SRC_TEX_DIR, filename)
    asset_name = os.path.splitext(filename)[0]
    uasset_path = f"{DEST_TEX_PATH}/{asset_name}"
    
    task = unreal.AssetImportTask()
    task.filename = file_path
    task.destination_path = DEST_TEX_PATH
    task.destination_name = asset_name
    task.replace_existing = True
    task.automated = True
    task.save = True
    
    asset_tools.import_asset_tasks([task])
    loaded = unreal.load_asset(uasset_path)
    if loaded:
        if asset_name.endswith('_N'):
            loaded.set_editor_property('compression_settings', unreal.TextureCompressionSettings.TC_NORMALMAP)
            loaded.set_editor_property('srgb', False)
        elif asset_name.endswith('_ORM'):
            loaded.set_editor_property('compression_settings', unreal.TextureCompressionSettings.TC_MASKS)
            loaded.set_editor_property('srgb', False)
        unreal.EditorAssetLibrary.save_loaded_asset(loaded)
        tex_count += 1
        print(f"[TEX OK] {uasset_path}")
    else:
        print(f"[TEX FAIL] {uasset_path}")

print(f"Total Textures Processed: {tex_count}")
print("==================================================")
print("=== BATCH IMPORT COMPLETED SUCCESSFULLY ===")
print("==================================================")
