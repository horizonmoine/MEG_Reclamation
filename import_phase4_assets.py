import unreal
import os

print("=== IMPORTING PHASE 4 TOOLS & MONSTER AUDIO ===")

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

# 1. Tools FBX
TOOLS_DIR = r"F:\MEG_Reclamation\RawAssets\FBX\Tools"
DEST_TOOLS = "/Game/Meshes/Tools"

tool_count = 0
for filename in os.listdir(TOOLS_DIR):
    if not filename.lower().endswith(".fbx"):
        continue
    file_path = os.path.join(TOOLS_DIR, filename)
    asset_name = os.path.splitext(filename)[0]
    
    task = unreal.AssetImportTask()
    task.filename = file_path
    task.destination_path = DEST_TOOLS
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
    loaded = unreal.load_asset(f"{DEST_TOOLS}/{asset_name}")
    if loaded:
        unreal.EditorAssetLibrary.save_loaded_asset(loaded)
        tool_count += 1
        print(f"[TOOL OK] {DEST_TOOLS}/{asset_name}")

# 2. Audio WAV
AUDIO_DIR = r"F:\MEG_Reclamation\RawAssets\Audio"
DEST_AUDIO = "/Game/Audio"

audio_count = 0
for filename in os.listdir(AUDIO_DIR):
    if not filename.lower().endswith(".wav"):
        continue
    file_path = os.path.join(AUDIO_DIR, filename)
    asset_name = os.path.splitext(filename)[0]
    
    task = unreal.AssetImportTask()
    task.filename = file_path
    task.destination_path = DEST_AUDIO
    task.destination_name = asset_name
    task.replace_existing = True
    task.automated = True
    task.save = True
    
    asset_tools.import_asset_tasks([task])
    loaded = unreal.load_asset(f"{DEST_AUDIO}/{asset_name}")
    if loaded:
        unreal.EditorAssetLibrary.save_loaded_asset(loaded)
        audio_count += 1
        print(f"[AUDIO OK] {DEST_AUDIO}/{asset_name}")

print(f"=== PHASE 4 ASSET IMPORT COMPLETE: {tool_count} TOOLS, {audio_count} AUDIO WAVES ===")
