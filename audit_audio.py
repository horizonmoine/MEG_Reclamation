import unreal
import json

def audit():
    output = {
        'attenuations': [],
        'submixes': [],
        'source_effects': [],
        'sound_classes': [],
        'cues': []
    }
    
    asset_reg = unreal.AssetRegistryHelpers.get_asset_registry()
    assets = asset_reg.get_all_assets()
    for a in assets:
        class_name = a.asset_class_path.asset_name.to_string()
        pkg_name = a.package_name.to_string()
        if class_name == 'SoundAttenuation':
            output['attenuations'].append(pkg_name)
        elif class_name == 'SoundSubmix':
            output['submixes'].append(pkg_name)
        elif class_name == 'SoundEffectSourcePresetChain':
            output['source_effects'].append(pkg_name)
        elif class_name == 'SoundClass':
            output['sound_classes'].append(pkg_name)
        elif class_name == 'SoundCue':
            output['cues'].append(pkg_name)

    with open('f:/MEG_Reclamation/audit_audio_out2.json', 'w') as f:
        json.dump(output, f, indent=4)

audit()
