import unreal

log = []

def configure_audio_systems():
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory_atten = unreal.SoundAttenuationFactory()
    
    # Update/Create SA_WalkieTalkie
    sa_walkie_path = "/Game/Audio/WalkieTalkie/SA_WalkieTalkie.SA_WalkieTalkie"
    sa_walkie = unreal.EditorAssetLibrary.load_asset(sa_walkie_path)
    if not sa_walkie:
        sa_walkie = asset_tools.create_asset("SA_WalkieTalkie", "/Game/Audio/WalkieTalkie", unreal.SoundAttenuation, factory_atten)
        log.append("Created SA_WalkieTalkie")
    else:
        log.append("Loaded SA_WalkieTalkie")
        
    if sa_walkie:
        walkie_settings = sa_walkie.attenuation
        walkie_settings.set_editor_property('enable_occlusion', True)
        walkie_settings.set_editor_property('occlusion_low_pass_filter_frequency', 3000.0)
        walkie_settings.set_editor_property('occlusion_volume_attenuation', 0.5)
        walkie_settings.set_editor_property('distance_algorithm', unreal.AttenuationDistanceModel.NATURAL_SOUND)
        walkie_settings.set_editor_property('attenuation_shape_extents', unreal.Vector(100.0, 0.0, 0.0))
        walkie_settings.set_editor_property('falloff_distance', 1500.0)
        unreal.EditorAssetLibrary.save_loaded_asset(sa_walkie)
        
    cue_factory = unreal.SoundCueFactory()
    sc_walkie_path = "/Game/Audio/WalkieTalkie/SC_WalkieTalkie_Distortion.SC_WalkieTalkie_Distortion"
    if not unreal.EditorAssetLibrary.does_asset_exist(sc_walkie_path):
        sc_walkie = asset_tools.create_asset("SC_WalkieTalkie_Distortion", "/Game/Audio/WalkieTalkie", unreal.SoundCue, cue_factory)
        log.append("Created WalkieTalkie Distortion Sound Cue")
        
    sc_walkie_noise_path = "/Game/Audio/WalkieTalkie/SC_WalkieTalkie_Noise.SC_WalkieTalkie_Noise"
    if not unreal.EditorAssetLibrary.does_asset_exist(sc_walkie_noise_path):
        sc_walkie_noise = asset_tools.create_asset("SC_WalkieTalkie_Noise", "/Game/Audio/WalkieTalkie", unreal.SoundCue, cue_factory)
        log.append("Created WalkieTalkie Noise Sound Cue")
        
    log.append("DONE_AUDIO_AUDIT")

try:
    configure_audio_systems()
except Exception as e:
    log.append(f"ERROR: {e}")

with open('f:/MEG_Reclamation/fix_audio_out2.txt', 'w') as f:
    for line in log:
        f.write(line + '\\n')
