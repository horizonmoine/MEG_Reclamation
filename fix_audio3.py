import unreal

log = []

def configure_audio_systems():
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    
    source_asset = "/Game/Audio/S_Terminal_Beep.S_Terminal_Beep"
    
    sc_walkie_path = "/Game/Audio/WalkieTalkie/SC_WalkieTalkie_Effect"
    if not unreal.EditorAssetLibrary.does_asset_exist(sc_walkie_path + ".SC_WalkieTalkie_Effect"):
        unreal.EditorAssetLibrary.duplicate_asset(source_asset, sc_walkie_path)
        log.append("Created WalkieTalkie Effect")
        
    sc_walkie_noise_path = "/Game/Audio/WalkieTalkie/SC_WalkieTalkie_Noise"
    if not unreal.EditorAssetLibrary.does_asset_exist(sc_walkie_noise_path + ".SC_WalkieTalkie_Noise"):
        unreal.EditorAssetLibrary.duplicate_asset(source_asset, sc_walkie_noise_path)
        log.append("Created WalkieTalkie Noise")
        
    log.append("DONE_AUDIO_AUDIT")

try:
    configure_audio_systems()
except Exception as e:
    log.append(f"ERROR: {e}")

with open('f:/MEG_Reclamation/fix_audio_out3.txt', 'w') as f:
    for line in log:
        f.write(line + '\\n')
