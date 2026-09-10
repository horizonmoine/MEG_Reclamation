import unreal

log = []

def configure_audio_systems():
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory_atten = unreal.SoundAttenuationFactory()
    
    # 1. Update SA_LiminalDefault (Diegetic Audio)
    sa_liminal_path = "/Game/Audio/SA_LiminalDefault.SA_LiminalDefault"
    sa_liminal = unreal.EditorAssetLibrary.load_asset(sa_liminal_path)
    if not sa_liminal:
        sa_liminal = asset_tools.create_asset("SA_LiminalDefault", "/Game/Audio", unreal.SoundAttenuation, factory_atten)
        log.append("Created SA_LiminalDefault")
    else:
        log.append("Loaded SA_LiminalDefault")
        
    attenuation_settings = sa_liminal.attenuation
    
    # Enable Occlusion (progressive low-pass filter through walls)
    attenuation_settings.set_editor_property('enable_occlusion', True)
    attenuation_settings.set_editor_property('occlusion_low_pass_filter_frequency', 2000.0) # progressive low pass
    attenuation_settings.set_editor_property('occlusion_volume_attenuation', 0.2)
    attenuation_settings.set_editor_property('use_complex_collision_for_occlusion', True)
    
    # Set realistic attenuation (Logarithmic or NaturalSound)
    attenuation_settings.set_editor_property('attenuation_shape', unreal.AttenuationShape.SPHERE)
    attenuation_settings.set_editor_property('distance_algorithm', unreal.AttenuationDistanceModel.NATURAL_SOUND)
    shape_extents = unreal.Vector(400.0, 0.0, 0.0) # Inner radius
    attenuation_settings.set_editor_property('attenuation_shape_extents', shape_extents)
    attenuation_settings.set_editor_property('falloff_distance', 3000.0)
    
    unreal.EditorAssetLibrary.save_loaded_asset(sa_liminal)

    # 2. Update/Create SA_ProximityVoIP
    sa_voip_path = "/Game/Audio/SA_ProximityVoIP.SA_ProximityVoIP"
    sa_voip = unreal.EditorAssetLibrary.load_asset(sa_voip_path)
    if not sa_voip:
        sa_voip = asset_tools.create_asset("SA_ProximityVoIP", "/Game/Audio", unreal.SoundAttenuation, factory_atten)
        log.append("Created SA_ProximityVoIP")
    else:
        log.append("Loaded SA_ProximityVoIP")
        
    voip_settings = sa_voip.attenuation
    voip_settings.set_editor_property('enable_occlusion', True)
    voip_settings.set_editor_property('occlusion_low_pass_filter_frequency', 2000.0)
    voip_settings.set_editor_property('occlusion_volume_attenuation', 0.1)
    
    # Natural Sound for VoIP
    voip_settings.set_editor_property('distance_algorithm', unreal.AttenuationDistanceModel.NATURAL_SOUND)
    voip_settings.set_editor_property('attenuation_shape_extents', unreal.Vector(200.0, 0.0, 0.0))
    voip_settings.set_editor_property('falloff_distance', 2000.0)
    
    unreal.EditorAssetLibrary.save_loaded_asset(sa_voip)

    # 3. Create Walkie-Talkie effects
    chain_path = "/Game/Audio/WalkieTalkie/SEC_WalkieTalkie.SEC_WalkieTalkie"
    if not unreal.EditorAssetLibrary.does_asset_exist(chain_path):
        chain_factory = unreal.SoundEffectSourcePresetChainFactory()
        sec_walkie = asset_tools.create_asset("SEC_WalkieTalkie", "/Game/Audio/WalkieTalkie", unreal.SoundEffectSourcePresetChain, chain_factory)
        log.append("Created SEC_WalkieTalkie")
        
    cue_factory = unreal.SoundCueFactory()
    sc_walkie_noise_path = "/Game/Audio/WalkieTalkie/SC_WalkieTalkie_Noise.SC_WalkieTalkie_Noise"
    if not unreal.EditorAssetLibrary.does_asset_exist(sc_walkie_noise_path):
        sc_walkie_noise = asset_tools.create_asset("SC_WalkieTalkie_Noise", "/Game/Audio/WalkieTalkie", unreal.SoundCue, cue_factory)
        log.append("Created WalkieTalkie Sound Cue for noise")
        
    log.append("DONE_AUDIO_AUDIT")

try:
    configure_audio_systems()
except Exception as e:
    log.append(f"ERROR: {e}")

with open('f:/MEG_Reclamation/fix_audio_out.txt', 'w') as f:
    for line in log:
        f.write(line + '\\n')
