import unreal
import os

def create_sound_attenuation():
    asset_name = "SA_LiminalDefault"
    package_path = "/Game/Audio"
    
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    
    # Check if exists
    if unreal.EditorAssetLibrary.does_asset_exist(f"{package_path}/{asset_name}"):
        return unreal.EditorAssetLibrary.load_asset(f"{package_path}/{asset_name}")

    factory = unreal.SoundAttenuationFactory()
    attenuation_asset = asset_tools.create_asset(asset_name, package_path, unreal.SoundAttenuation, factory)
    
    settings = unreal.SoundAttenuationSettings()
    settings.attenuation_shape_extents = unreal.Vector(300.0, 0.0, 0.0) # inner radius 300
    settings.falloff_distance = 2000.0
    settings.distance_algorithm = unreal.AttenuationDistanceModel.NATURAL_SOUND
    
    attenuation_asset.attenuation = settings
    
    unreal.EditorAssetLibrary.save_asset(attenuation_asset.get_path_name())
    return attenuation_asset

def import_wavs():
    raw_dir = 'F:/MEG_Reclamation/RawAudio'
    if not os.path.exists(raw_dir):
        unreal.log_error(f"Directory {raw_dir} does not exist.")
        return

    wav_files = [f for f in os.listdir(raw_dir) if f.endswith('.wav')]
    tasks = []
    
    for f in wav_files:
        task = unreal.AssetImportTask()
        task.filename = os.path.join(raw_dir, f)
        task.destination_path = '/Game/Audio'
        task.destination_name = f.replace('.wav', '')
        task.automated = True
        task.save = True
        task.options = unreal.FbxImportUI() # Can just use default for sound
        tasks.append(task)
        
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

    # Now apply attenuation to all
    attenuation = create_sound_attenuation()
    
    for f in wav_files:
        asset_name = f.replace('.wav', '')
        asset_path = f"/Game/Audio/{asset_name}.{asset_name}"
        sound_wave = unreal.EditorAssetLibrary.load_asset(asset_path)
        if sound_wave:
            sound_wave.attenuation_settings = attenuation
            unreal.EditorAssetLibrary.save_asset(asset_path)

if __name__ == '__main__':
    import_wavs()
    print("Audio import and attenuation setup complete.")
