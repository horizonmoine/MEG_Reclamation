import unreal
import os

print("=== IMPORTING AUDIO SFX INTO UNREAL ENGINE ===")
SRC_DIR = r"F:\MEG_Reclamation\RawAssets\Audio"
DEST_PATH = "/Game/Audio"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

count = 0
for filename in os.listdir(SRC_DIR):
    if not filename.lower().endswith(".wav"):
        continue
    file_path = os.path.join(SRC_DIR, filename)
    asset_name = os.path.splitext(filename)[0]
    
    task = unreal.AssetImportTask()
    task.filename = file_path
    task.destination_path = DEST_PATH
    task.destination_name = asset_name
    task.replace_existing = True
    task.automated = True
    task.save = True
    
    asset_tools.import_asset_tasks([task])
    loaded = unreal.load_asset(f"{DEST_PATH}/{asset_name}")
    if loaded:
        unreal.EditorAssetLibrary.save_loaded_asset(loaded)
        count += 1
        print(f"[AUDIO OK] {DEST_PATH}/{asset_name}")
    else:
        print(f"[AUDIO FAIL] {DEST_PATH}/{asset_name}")

print(f"=== AUDIO IMPORT COMPLETE: {count} SOUND WAVES IMPORTED ===")
