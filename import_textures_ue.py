import unreal
import os

print("=== IMPORTING BACKROOMS TEXTURES INTO UNREAL ENGINE ===")
SRC_DIR = r"F:\MEG_Reclamation\Content\Textures\Backrooms"
DEST_PATH = "/Game/Textures/Backrooms"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

for filename in os.listdir(SRC_DIR):
    if not filename.lower().endswith(".png"):
        continue
        
    file_path = os.path.join(SRC_DIR, filename)
    asset_name = os.path.splitext(filename)[0]
    
    # Check if uasset already exists
    uasset_path = f"{DEST_PATH}/{asset_name}"
    
    task = unreal.AssetImportTask()
    task.filename = file_path
    task.destination_path = DEST_PATH
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
        print(f"Successfully imported and configured PBR texture: {uasset_path}")
    else:
        print(f"Failed to import: {uasset_path}")


print("=== TEXTURE IMPORT COMPLETE ===")
