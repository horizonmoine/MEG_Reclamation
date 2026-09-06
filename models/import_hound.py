import unreal

at = unreal.AssetToolsHelpers.get_asset_tools()
t = unreal.AssetImportTask()
t.filename = 'F:/MEG_Reclamation/models/hound_extracted/Meshy_AI_Backroom_Creature_Enc_0824101528_texture_fbx/Meshy_AI_Backroom_Creature_Enc_0824101528_texture.fbx'
t.destination_path = '/Game/Meshes/Hound'
t.destination_name = 'SM_Hound'
t.automated = True
t.save = True
t.replace_existing = True
at.import_asset_tasks([t])

ar = unreal.AssetRegistryHelpers.get_asset_registry()
for a in ar.get_assets_by_path('/Game/Meshes/Hound', True):
    print('IMPORTED:', str(a.package_name), str(a.asset_class_path.asset_name))

mesh = unreal.load_object(None, '/Game/Meshes/Hound/SM_Hound.SM_Hound')
if mesh:
    box = mesh.get_bounding_box()
    size = box.get_size()
    print('BOUNDS:', size.x, size.y, size.z)
