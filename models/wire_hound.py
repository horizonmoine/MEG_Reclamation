import unreal

FBX_PATH = 'F:/MEG_Reclamation/models/hound_extracted/Meshy_AI_Backroom_Creature_Enc_0824101528_texture_fbx/Meshy_AI_Backroom_Creature_Enc_0824101528_texture.fbx'
DEST_PATH = '/Game/Meshes/Hound'
MESH_PATH = '/Game/Meshes/Hound/SM_Hound.SM_Hound'
BP_PATH = '/Game/AI/BP_Hound.BP_Hound'
TARGET_HEIGHT_CM = 110.0


def log(msg):
    unreal.log('WIRE_HOUND: %s' % msg)


def import_mesh():
    task = unreal.AssetImportTask()
    task.filename = FBX_PATH
    task.destination_path = DEST_PATH
    task.destination_name = 'SM_Hound'
    task.automated = True
    task.save = True
    task.replace_existing = True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return unreal.load_object(None, MESH_PATH)


def wire_blueprint(mesh, scale):
    bp = unreal.load_object(None, BP_PATH)
    if not bp:
        log('ERREUR: BP_Hound introuvable')
        return False

    generated_class = bp.generated_class()
    cdo = unreal.get_default_object(generated_class)
    body_mesh = cdo.get_editor_property('BodyMesh')
    body_mesh.set_editor_property('static_mesh', mesh)
    body_mesh.set_editor_property('relative_scale_3d', unreal.Vector(scale, scale, scale))

    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    unreal.EditorAssetLibrary.save_asset('/Game/AI/BP_Hound', only_if_is_dirty=False)
    log('BP_Hound cable (echelle %.4f)' % scale)
    return True


mesh = import_mesh()
if not mesh:
    log('ERREUR: import SM_Hound echoue')
else:
    box = mesh.get_bounding_box()
    size = box.max - box.min
    log('bounds: %.1f x %.1f x %.1f cm' % (size.x, size.y, size.z))
    scale = TARGET_HEIGHT_CM / max(size.z, 1.0)
    ok = wire_blueprint(mesh, scale)
    log('RESULTAT: %s' % ('OK' if ok else 'ECHEC'))
