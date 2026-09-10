import unreal

log = []

def apply_attenuation():
    sa_walkie_path = "/Game/Audio/WalkieTalkie/SA_WalkieTalkie.SA_WalkieTalkie"
    sa_walkie = unreal.EditorAssetLibrary.load_asset(sa_walkie_path)
    
    sc_paths = [
        "/Game/Audio/WalkieTalkie/SC_WalkieTalkie_Effect.SC_WalkieTalkie_Effect",
        "/Game/Audio/WalkieTalkie/SC_WalkieTalkie_Noise.SC_WalkieTalkie_Noise"
    ]
    
    for path in sc_paths:
        asset = unreal.EditorAssetLibrary.load_asset(path)
        if asset:
            # For SoundBase (which both SoundWave and SoundCue inherit from)
            try:
                asset.set_editor_property('attenuation_settings', sa_walkie)
                unreal.EditorAssetLibrary.save_loaded_asset(asset)
                log.append(f"Applied attenuation to {path}")
            except Exception as e:
                log.append(f"Failed to apply to {path}: {e}")

try:
    apply_attenuation()
except Exception as e:
    log.append(f"ERROR: {e}")

with open('f:/MEG_Reclamation/fix_audio_out4.txt', 'w') as f:
    for line in log:
        f.write(line + '\\n')
