import bpy
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=F:/MEG_Reclamation/models/hound.fbx)
print(=== HOUND FBX INSPECTION ===)
print(Objects:, [(o.name, o.type) for o in bpy.data.objects])
print(Armatures:, [a.name for a in bpy.data.armatures])
for arm in bpy.data.armatures:
    print(Bones in, arm.name, :, [b.name for b in arm.bones])
print(Actions:, [a.name for a in bpy.data.actions])