import unreal
import os

print('=== STARTING BATCH UNREAL ASSET IMPORT ===')

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

SRC_BASE = r'F:\MEG_Reclamation\RawAssets\FBX'
MAPPINGS = [
    (os.path.join(SRC_BASE, 'Modular'), '/Game/Meshes/Modular'),
    (os.path.join(SRC_BASE, 'Props'), '/Game/Meshes/Props'),
    (os.path.join(SRC_BASE, 'Puzzles'), '/Game/Meshes/Puzzles'),
    (os.path.join(SRC_BASE, 'Tools'), '/Game/Meshes/Tools'),
]

total_imported = 0

for src_dir, dest_path in MAPPINGS:
    if not os.path.exists(src_dir):
        continue
        
    for filename in os.listdir(src_dir):
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
        options.import_materials = True
        options.import_as_skeletal = False
        
        # Static mesh options
        options.static_mesh_import_data.combine_meshes = True
        options.static_mesh_import_data.auto_generate_collision = True
        options.static_mesh_import_data.generate_lightmap_u_vs = True
        
        task.options = options
        
        asset_tools.import_asset_tasks([task])
        
        loaded = unreal.load_asset(f'{dest_path}/{asset_name}')
        if loaded:
            unreal.EditorAssetLibrary.save_loaded_asset(loaded)
            total_imported += 1
            print(f'-> Successfully imported and saved: {dest_path}/{asset_name}')
        else:
            print(f'-> FAILED to import: {dest_path}/{asset_name}')

print(f'=== BATCH IMPORT COMPLETE: {total_imported} ASSETS IMPORTED ===')
