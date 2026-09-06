import unreal
import sys

entities = ["Partygoer", "Smiler", "Clump", "Deathmoth", "Duller", "Jerry", "Skinwalker", "Watcher", "Wretch"]

for e in entities:
    bb_path = f"/Game/AI/BB_{e}"
    bt_path = f"/Game/AI/BT_{e}"
    
    if not unreal.EditorAssetLibrary.does_asset_exist(bb_path):
        unreal.EditorAssetLibrary.duplicate_asset("/Game/AI/BB_Hound", bb_path)
    
    if not unreal.EditorAssetLibrary.does_asset_exist(bt_path):
        unreal.EditorAssetLibrary.duplicate_asset("/Game/AI/BT_Hound", bt_path)

unreal.EditorAssetLibrary.save_directory("/Game/AI")
print("AI assets created successfully!")
sys.exit(0)
