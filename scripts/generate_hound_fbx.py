"""
Generates complete SK_Hound FBX asset with:
- Measured scale and orientation matching Unreal Character standards (1 unit = 1 cm, ~100cm height, 180cm length)
- 20-bone armature with AttackSocket
- PBR material M_Hound with Albedo texture
- Animations:
  - Hound_Idle (looping breathing)
  - Hound_Walk (looping quadruped walk)
  - Hound_Run (course - looping quadruped sprint)
  - Hound_Attack (full 3-phase: anticipation -> impact -> recovery)
  - Hound_Anticipation (anticipation coil)
  - Hound_Impact (lunge and bite snap)
  - Hound_Recovery (landing and reset)
  - Hound_Death (collapse)
"""

import bpy
import bmesh
import math
import os

OUTPUT_FBX = r"F:\MEG_Reclamation\RawAssets\FBX\Bestiary\SK_Hound.fbx"
TEXTURE_PATH = r"F:\MEG_Reclamation\models\hound_extracted\Meshy_AI_Backroom_Creature_Enc_0824101528_texture_fbx\Meshy_AI_Backroom_Creature_Enc_0824101528_texture.png"
SOURCE_FBX = r"F:\MEG_Reclamation\models\hound_extracted\Meshy_AI_Backroom_Creature_Enc_0824101528_texture_fbx\Meshy_AI_Backroom_Creature_Enc_0824101528_texture.fbx"

# Reset scene
bpy.ops.wm.read_factory_settings(use_empty=True)
for c in list(bpy.data.collections):
    bpy.data.collections.remove(c)
col = bpy.data.collections.new("HoundExport")
bpy.context.scene.collection.children.link(col)
bpy.context.view_layer.active_layer_collection = bpy.context.view_layer.layer_collection.children["HoundExport"]

# Import source mesh or build procedural
mesh_obj = None
if os.path.exists(SOURCE_FBX):
    bpy.ops.import_scene.fbx(filepath=SOURCE_FBX)
    for obj in bpy.context.selected_objects:
        if obj.type == 'MESH':
            mesh_obj = obj
            break

if not mesh_obj:
    print("Fallback: building procedural hound mesh...")
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.6))
    mesh_obj = bpy.context.active_object
    mesh_obj.name = "SKM_Hound"
    mesh_obj.scale = (0.45, 1.8, 0.8)
    bpy.ops.object.transform_apply(scale=True)
else:
    mesh_obj.name = "SKM_Hound"
    bpy.ops.object.select_all(action='DESELECT')
    mesh_obj.select_set(True)
    bpy.context.view_layer.objects.active = mesh_obj
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)

# Ensure PBR material setup
mat = bpy.data.materials.new(name="M_Hound")
mat.use_nodes = True
nodes = mat.node_tree.nodes
nodes.clear()

bsdf = nodes.new(type='ShaderNodeBsdfPrincipled')
bsdf.location = (200, 0)
bsdf.inputs['Roughness'].default_value = 0.7
bsdf.inputs['Metallic'].default_value = 0.0

output = nodes.new(type='ShaderNodeOutputMaterial')
output.location = (500, 0)
mat.node_tree.links.new(bsdf.outputs['BSDF'], output.inputs['Surface'])

if os.path.exists(TEXTURE_PATH):
    tex_img = nodes.new(type='ShaderNodeTexImage')
    tex_img.location = (-200, 0)
    img = bpy.data.images.load(TEXTURE_PATH)
    tex_img.image = img
    mat.node_tree.links.new(tex_img.outputs['Color'], bsdf.inputs['Base Color'])

mesh_obj.data.materials.clear()
mesh_obj.data.materials.append(mat)

# Create Armature
arm_data = bpy.data.armatures.new("SK_Hound_Skeleton")
arm_obj = bpy.data.objects.new("Armature_Hound", arm_data)
bpy.context.collection.objects.link(arm_obj)
bpy.context.view_layer.objects.active = arm_obj
bpy.ops.object.mode_set(mode='EDIT')

bones = {}
def make_bone(name, head, tail, parent_name=None):
    b = arm_data.edit_bones.new(name)
    b.head = head
    b.tail = tail
    if parent_name and parent_name in bones:
        b.parent = bones[parent_name]
    bones[name] = b
    return b

# Bone hierarchy
make_bone("root", (0, 0, 0), (0, 0, 0.15))
make_bone("pelvis", (0, -0.45, 0.65), (0, -0.20, 0.68), "root")
make_bone("spine_01", (0, -0.20, 0.68), (0, 0.10, 0.70), "pelvis")
make_bone("chest", (0, 0.10, 0.70), (0, 0.35, 0.72), "spine_01")
make_bone("neck", (0, 0.35, 0.72), (0, 0.55, 0.76), "chest")
make_bone("head", (0, 0.55, 0.76), (0, 0.78, 0.80), "neck")
make_bone("jaw", (0, 0.60, 0.74), (0, 0.80, 0.72), "head")
make_bone("AttackSocket", (0, 0.78, 0.75), (0, 0.90, 0.75), "jaw")

# Front Left / Right
make_bone("clavicle_l", (-0.12, 0.35, 0.68), (-0.18, 0.35, 0.50), "chest")
make_bone("forearm_l", (-0.18, 0.35, 0.50), (-0.18, 0.35, 0.25), "clavicle_l")
make_bone("paw_fl", (-0.18, 0.35, 0.25), (-0.18, 0.38, 0.0), "forearm_l")

make_bone("clavicle_r", (0.12, 0.35, 0.68), (0.18, 0.35, 0.50), "chest")
make_bone("forearm_r", (0.18, 0.35, 0.50), (0.18, 0.35, 0.25), "clavicle_r")
make_bone("paw_fr", (0.18, 0.35, 0.25), (0.18, 0.38, 0.0), "forearm_r")

# Hind Left / Right
make_bone("thigh_l", (-0.15, -0.45, 0.65), (-0.18, -0.45, 0.35), "pelvis")
make_bone("calf_l", (-0.18, -0.45, 0.35), (-0.18, -0.40, 0.15), "thigh_l")
make_bone("paw_bl", (-0.18, -0.40, 0.15), (-0.18, -0.38, 0.0), "calf_l")

make_bone("thigh_r", (0.15, -0.45, 0.65), (0.18, -0.45, 0.35), "pelvis")
make_bone("calf_r", (0.18, -0.45, 0.35), (0.18, -0.40, 0.15), "thigh_r")
make_bone("paw_br", (0.18, -0.40, 0.15), (0.18, -0.38, 0.0), "calf_r")

bpy.ops.object.mode_set(mode='OBJECT')

# Skin mesh to armature with auto weights
bpy.ops.object.select_all(action='DESELECT')
mesh_obj.select_set(True)
arm_obj.select_set(True)
bpy.context.view_layer.objects.active = arm_obj
bpy.ops.object.parent_set(type='ARMATURE_AUTO')

def add_nla_track(armature_obj, action):
    if not armature_obj.animation_data:
        armature_obj.animation_data_create()
    track = armature_obj.animation_data.nla_tracks.new()
    track.name = action.name
    strip = track.strips.new(action.name, int(action.frame_range[0]), action)
    strip.action = action

arm_obj.animation_data_create()

# ----------------------------------------------------
# 1. Hound_Idle (60 frames, 2s @ 30fps)
# ----------------------------------------------------
act_idle = bpy.data.actions.new("Hound_Idle")
arm_obj.animation_data.action = act_idle
for f in (1, 15, 30, 45, 60):
    t = (f - 1) / 59.0
    z_chest = math.sin(t * math.pi * 2.0) * 0.03
    arm_obj.pose.bones["chest"].location = (0, 0, z_chest)
    arm_obj.pose.bones["chest"].keyframe_insert(data_path="location", frame=f)
    
    pitch_head = math.sin(t * math.pi * 2.0) * 0.04
    arm_obj.pose.bones["head"].rotation_euler = (pitch_head, 0, 0)
    arm_obj.pose.bones["head"].keyframe_insert(data_path="rotation_euler", frame=f)

    jaw_rot = max(0.0, math.sin(t * math.pi * 2.0) * 0.06)
    arm_obj.pose.bones["jaw"].rotation_euler = (jaw_rot, 0, 0)
    arm_obj.pose.bones["jaw"].keyframe_insert(data_path="rotation_euler", frame=f)
add_nla_track(arm_obj, act_idle)

# ----------------------------------------------------
# 2. Hound_Walk (30 frames, 1.0s @ 30fps)
# ----------------------------------------------------
act_walk = bpy.data.actions.new("Hound_Walk")
arm_obj.animation_data.action = act_walk
for f in range(1, 31):
    phase = (f - 1) / 30.0 * math.pi * 2.0
    # Diagonal trot gait: (paw_fl + paw_br) vs (paw_fr + paw_bl)
    arm_obj.pose.bones["paw_fl"].location = (0, math.sin(phase) * 0.18, max(0.0, math.cos(phase) * 0.09))
    arm_obj.pose.bones["paw_fl"].keyframe_insert(data_path="location", frame=f)
    arm_obj.pose.bones["paw_br"].location = (0, math.sin(phase) * 0.18, max(0.0, math.cos(phase) * 0.09))
    arm_obj.pose.bones["paw_br"].keyframe_insert(data_path="location", frame=f)
    
    arm_obj.pose.bones["paw_fr"].location = (0, -math.sin(phase) * 0.18, max(0.0, -math.cos(phase) * 0.09))
    arm_obj.pose.bones["paw_fr"].keyframe_insert(data_path="location", frame=f)
    arm_obj.pose.bones["paw_bl"].location = (0, -math.sin(phase) * 0.18, max(0.0, -math.cos(phase) * 0.09))
    arm_obj.pose.bones["paw_bl"].keyframe_insert(data_path="location", frame=f)

    # Spine undulating
    arm_obj.pose.bones["spine_01"].rotation_euler = (0, math.sin(phase) * 0.05, math.sin(phase) * 0.04)
    arm_obj.pose.bones["spine_01"].keyframe_insert(data_path="rotation_euler", frame=f)
add_nla_track(arm_obj, act_walk)

# ----------------------------------------------------
# 3. Hound_Run (course - 18 frames, 0.6s @ 30fps)
# Aggressive gallop, bounding motion, spine compression/extension
# ----------------------------------------------------
act_run = bpy.data.actions.new("Hound_Run")
arm_obj.animation_data.action = act_run
for f in range(1, 19):
    phase = (f - 1) / 18.0 * math.pi * 2.0
    
    # Root & chest bounce
    bounce_z = math.sin(phase * 2.0) * 0.08
    arm_obj.pose.bones["root"].location = (0, 0, bounce_z)
    arm_obj.pose.bones["root"].keyframe_insert(data_path="location", frame=f)
    
    # Spine flexion
    spine_pitch = math.sin(phase) * 0.12
    arm_obj.pose.bones["spine_01"].rotation_euler = (spine_pitch, 0, 0)
    arm_obj.pose.bones["spine_01"].keyframe_insert(data_path="rotation_euler", frame=f)

    # Front paws reach and strike
    arm_obj.pose.bones["paw_fl"].location = (0, math.sin(phase) * 0.35, max(0.0, math.cos(phase) * 0.16))
    arm_obj.pose.bones["paw_fl"].keyframe_insert(data_path="location", frame=f)
    arm_obj.pose.bones["paw_fr"].location = (0, math.sin(phase + 0.3) * 0.35, max(0.0, math.cos(phase + 0.3) * 0.16))
    arm_obj.pose.bones["paw_fr"].keyframe_insert(data_path="location", frame=f)
    
    # Hind paws push
    arm_obj.pose.bones["paw_bl"].location = (0, -math.sin(phase) * 0.32, max(0.0, -math.cos(phase) * 0.14))
    arm_obj.pose.bones["paw_bl"].keyframe_insert(data_path="location", frame=f)
    arm_obj.pose.bones["paw_br"].location = (0, -math.sin(phase + 0.3) * 0.32, max(0.0, -math.cos(phase + 0.3) * 0.14))
    arm_obj.pose.bones["paw_br"].keyframe_insert(data_path="location", frame=f)

    # Head low and aggressive
    arm_obj.pose.bones["head"].rotation_euler = (-0.15 + math.sin(phase) * 0.06, 0, 0)
    arm_obj.pose.bones["head"].keyframe_insert(data_path="rotation_euler", frame=f)
    arm_obj.pose.bones["jaw"].rotation_euler = (0.15, 0, 0)
    arm_obj.pose.bones["jaw"].keyframe_insert(data_path="rotation_euler", frame=f)
add_nla_track(arm_obj, act_run)

# ----------------------------------------------------
# 4. Hound_Attack (36 frames, 1.2s @ 30fps)
# Phase 1 - Anticipation (f 1-12): coil back, raise chest, jaw opens wide
# Phase 2 - Impact (f 13-18, peak impact at f 15): fierce forward lunge, jaw clamp
# Phase 3 - Récupération (f 19-36): landing, settling back to 4 legs
# ----------------------------------------------------
act_attack = bpy.data.actions.new("Hound_Attack")
arm_obj.animation_data.action = act_attack

keyframes_attack = [
    # (frame, chest_loc_y, chest_loc_z, head_pitch, jaw_rot, pelvis_z)
    (1,  0.0,   0.0,   0.0,   0.0,   0.0),    # rest
    (6, -0.15,  0.05,  0.15,  0.25, -0.05),  # anticipation coil starts
    (10,-0.25,  0.12,  0.25,  0.60, -0.10),  # maximum anticipation coil, jaw wide open
    (13, 0.15,  0.08, -0.10,  0.50,  0.0),   # lunge begins forward
    (15, 0.55, -0.05, -0.25, -0.20,  0.05),  # IMPACT HIT: max lunge forward, jaw snap shut!
    (18, 0.45, -0.08, -0.20, -0.10,  0.0),   # follow-through bite grip
    (24, 0.25, -0.04, -0.05,  0.0,  -0.02),  # recovery landing
    (30, 0.10, -0.02,  0.0,   0.05,  0.0),   # steadying paws
    (36, 0.0,   0.0,   0.0,   0.0,   0.0)    # fully recovered to neutral
]

for f, cy, cz, hp, jr, pz in keyframes_attack:
    arm_obj.pose.bones["chest"].location = (0, cy, cz)
    arm_obj.pose.bones["chest"].keyframe_insert(data_path="location", frame=f)
    arm_obj.pose.bones["head"].rotation_euler = (hp, 0, 0)
    arm_obj.pose.bones["head"].keyframe_insert(data_path="rotation_euler", frame=f)
    arm_obj.pose.bones["jaw"].rotation_euler = (jr, 0, 0)
    arm_obj.pose.bones["jaw"].keyframe_insert(data_path="rotation_euler", frame=f)
    arm_obj.pose.bones["pelvis"].location = (0, 0, pz)
    arm_obj.pose.bones["pelvis"].keyframe_insert(data_path="location", frame=f)
add_nla_track(arm_obj, act_attack)

# ----------------------------------------------------
# 5. Hound_Anticipation (12 frames)
# ----------------------------------------------------
act_anticipation = bpy.data.actions.new("Hound_Anticipation")
arm_obj.animation_data.action = act_anticipation
for f in (1, 6, 12):
    t = (f - 1) / 11.0
    arm_obj.pose.bones["chest"].location = (0, -0.25 * t, 0.12 * t)
    arm_obj.pose.bones["chest"].keyframe_insert(data_path="location", frame=f)
    arm_obj.pose.bones["head"].rotation_euler = (0.25 * t, 0, 0)
    arm_obj.pose.bones["head"].keyframe_insert(data_path="rotation_euler", frame=f)
    arm_obj.pose.bones["jaw"].rotation_euler = (0.60 * t, 0, 0)
    arm_obj.pose.bones["jaw"].keyframe_insert(data_path="rotation_euler", frame=f)
add_nla_track(arm_obj, act_anticipation)

# ----------------------------------------------------
# 6. Hound_Impact (10 frames)
# ----------------------------------------------------
act_impact = bpy.data.actions.new("Hound_Impact")
arm_obj.animation_data.action = act_impact
for f in (1, 5, 10):
    t = (f - 1) / 9.0
    cy = 0.55 if f == 5 else (0.2 if f == 1 else 0.4)
    jr = -0.25 if f == 5 else (0.4 if f == 1 else -0.1)
    arm_obj.pose.bones["chest"].location = (0, cy, -0.05 * t)
    arm_obj.pose.bones["chest"].keyframe_insert(data_path="location", frame=f)
    arm_obj.pose.bones["jaw"].rotation_euler = (jr, 0, 0)
    arm_obj.pose.bones["jaw"].keyframe_insert(data_path="rotation_euler", frame=f)
add_nla_track(arm_obj, act_impact)

# ----------------------------------------------------
# 7. Hound_Recovery (16 frames)
# ----------------------------------------------------
act_recup = bpy.data.actions.new("Hound_Recovery")
arm_obj.animation_data.action = act_recup
for f in (1, 8, 16):
    t = (16 - f) / 15.0
    arm_obj.pose.bones["chest"].location = (0, 0.35 * t, -0.05 * t)
    arm_obj.pose.bones["chest"].keyframe_insert(data_path="location", frame=f)
    arm_obj.pose.bones["head"].rotation_euler = (-0.15 * t, 0, 0)
    arm_obj.pose.bones["head"].keyframe_insert(data_path="rotation_euler", frame=f)
    arm_obj.pose.bones["jaw"].rotation_euler = (0, 0, 0)
    arm_obj.pose.bones["jaw"].keyframe_insert(data_path="rotation_euler", frame=f)
add_nla_track(arm_obj, act_recup)

# ----------------------------------------------------
# 8. Hound_Death (35 frames)
# ----------------------------------------------------
act_death = bpy.data.actions.new("Hound_Death")
arm_obj.animation_data.action = act_death
for f, z_drop, pitch in [(1, 0, 0), (14, -0.3, 0.3), (35, -0.55, 0.5)]:
    arm_obj.pose.bones["pelvis"].location = (0, 0, z_drop)
    arm_obj.pose.bones["pelvis"].rotation_euler = (pitch, 0, 0.3)
    arm_obj.pose.bones["pelvis"].keyframe_insert(data_path="location", frame=f)
    arm_obj.pose.bones["pelvis"].keyframe_insert(data_path="rotation_euler", frame=f)
add_nla_track(arm_obj, act_death)

# Export FBX
bpy.ops.object.select_all(action='DESELECT')
mesh_obj.select_set(True)
arm_obj.select_set(True)
bpy.context.view_layer.objects.active = arm_obj

print(f"Exporting Hound FBX to {OUTPUT_FBX}...")
bpy.ops.export_scene.fbx(
    filepath=OUTPUT_FBX,
    check_existing=False,
    use_selection=True,
    global_scale=1.0,
    apply_unit_scale=True,
    apply_scale_options='FBX_SCALE_NONE',
    bake_space_transform=False,
    object_types={'ARMATURE', 'MESH'},
    use_mesh_modifiers=True,
    mesh_smooth_type='FACE',
    add_leaf_bones=False,
    primary_bone_axis='Y',
    secondary_bone_axis='X',
    armature_nodetype='NULL',
    bake_anim=True,
    bake_anim_use_all_bones=True,
    bake_anim_use_nla_strips=True,
    bake_anim_use_all_actions=True,
    bake_anim_step=1.0,
    bake_anim_simplify_factor=0.0
)

print(f"Exported successfully! Size: {os.path.getsize(OUTPUT_FBX)} bytes")
