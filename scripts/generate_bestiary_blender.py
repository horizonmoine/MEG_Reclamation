"""
Procedural Bestiary Generator for M.E.G. : Reclamation
Compatible with Blender 5.2.1 LTS
Generates rigged, keyframed skeletal meshes for:
- Hound (Quadruped predator, rigged using Meshy model)
- Deathmoth (Aerial insectoid flier)
- Clump (Amorphous multi-limb ambush mass)
- Jerry (Anomalous psionic avian)

Exports ready-to-use FBX assets to RawAssets/FBX/Bestiary/
"""

import bpy
import bmesh
import math
import os
import sys

OUTPUT_DIR = r"F:\MEG_Reclamation\RawAssets\FBX\Bestiary"
os.makedirs(OUTPUT_DIR, exist_ok=True)

HOUND_SOURCE_FBX = r"F:\MEG_Reclamation\models\hound_extracted\Meshy_AI_Backroom_Creature_Enc_0824101528_texture_fbx\Meshy_AI_Backroom_Creature_Enc_0824101528_texture.fbx"

def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    for c in list(bpy.data.collections):
        bpy.data.collections.remove(c)
    col = bpy.data.collections.new("BestiaryExport")
    bpy.context.scene.collection.children.link(col)
    bpy.context.view_layer.active_layer_collection = bpy.context.view_layer.layer_collection.children["BestiaryExport"]
    return col

def export_fbx(filepath, objects):
    bpy.ops.object.select_all(action='DESELECT')
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    
    print(f"Exporting FBX to: {filepath}")
    bpy.ops.export_scene.fbx(
        filepath=filepath,
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
    print(f"Export complete: {filepath} ({os.path.getsize(filepath)} bytes)")

def add_nla_track(armature_obj, action):
    if not armature_obj.animation_data:
        armature_obj.animation_data_create()
    track = armature_obj.animation_data.nla_tracks.new()
    track.name = action.name
    strip = track.strips.new(action.name, int(action.frame_range[0]), action)
    strip.action = action

# ==============================================================================
# 1. HOUND (Quadruped Predator)
# ==============================================================================
def build_hound():
    print("=== Building Hound Rig & Animations ===")
    reset_scene()
    
    mesh_obj = None
    if os.path.exists(HOUND_SOURCE_FBX):
        bpy.ops.import_scene.fbx(filepath=HOUND_SOURCE_FBX)
        for obj in bpy.context.selected_objects:
            if obj.type == 'MESH':
                mesh_obj = obj
                break
    
    if not mesh_obj:
        print("Creating procedural Hound mesh fallback...")
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.6))
        mesh_obj = bpy.context.active_object
        mesh_obj.name = "SKM_Hound"
        mesh_obj.scale = (0.45, 1.8, 0.8)
        bpy.ops.object.transform_apply(scale=True)
    else:
        mesh_obj.name = "SKM_Hound"
        # Orient facing +X or aligned
        bpy.ops.object.select_all(action='DESELECT')
        mesh_obj.select_set(True)
        bpy.context.view_layer.objects.active = mesh_obj
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    
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
    
    # Hound Armature hierarchy (cm converted to meters, Y forward/back)
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
    
    # Parent mesh to armature with auto weights
    bpy.ops.object.select_all(action='DESELECT')
    mesh_obj.select_set(True)
    arm_obj.select_set(True)
    bpy.context.view_layer.objects.active = arm_obj
    bpy.ops.object.parent_set(type='ARMATURE_AUTO')
    
    # Create 4 Keyframed Actions
    arm_obj.animation_data_create()
    
    # Action 1: Hound_Idle (60 frames, 2s)
    act_idle = bpy.data.actions.new("Hound_Idle")
    arm_obj.animation_data.action = act_idle
    for f in (1, 30, 60):
        t = (f - 1) / 59.0
        z_off = math.sin(t * math.pi * 2.0) * 0.03
        arm_obj.pose.bones["chest"].location = (0, 0, z_off)
        arm_obj.pose.bones["chest"].keyframe_insert(data_path="location", frame=f)
        jaw_rot = math.sin(t * math.pi * 2.0) * 0.05
        arm_obj.pose.bones["jaw"].rotation_euler = (jaw_rot, 0, 0)
        arm_obj.pose.bones["jaw"].keyframe_insert(data_path="rotation_euler", frame=f)
    add_nla_track(arm_obj, act_idle)
    
    # Action 2: Hound_Walk (24 frames, 0.8s)
    act_walk = bpy.data.actions.new("Hound_Walk")
    arm_obj.animation_data.action = act_walk
    for f in range(1, 25):
        phase = (f - 1) / 24.0 * math.pi * 2.0
        arm_obj.pose.bones["paw_fl"].location = (0, math.sin(phase) * 0.15, max(0.0, math.cos(phase) * 0.08))
        arm_obj.pose.bones["paw_fl"].keyframe_insert(data_path="location", frame=f)
        arm_obj.pose.bones["paw_br"].location = (0, math.sin(phase) * 0.15, max(0.0, math.cos(phase) * 0.08))
        arm_obj.pose.bones["paw_br"].keyframe_insert(data_path="location", frame=f)
        
        arm_obj.pose.bones["paw_fr"].location = (0, -math.sin(phase) * 0.15, max(0.0, -math.cos(phase) * 0.08))
        arm_obj.pose.bones["paw_fr"].keyframe_insert(data_path="location", frame=f)
        arm_obj.pose.bones["paw_bl"].location = (0, -math.sin(phase) * 0.15, max(0.0, -math.cos(phase) * 0.08))
        arm_obj.pose.bones["paw_bl"].keyframe_insert(data_path="location", frame=f)
    add_nla_track(arm_obj, act_walk)
    
    # Action 3: Hound_Attack (30 frames, 1.0s)
    act_attack = bpy.data.actions.new("Hound_Attack")
    arm_obj.animation_data.action = act_attack
    # Anticipation (f=1-8), lunge (f=9-14), bite at f=12, recover (f=15-30)
    for f, fwd, z, jaw_angle in [(1, 0, 0, 0), (8, -0.1, -0.05, 0.1), (12, 0.45, 0.1, -0.35), (18, 0.3, 0.0, 0.0), (30, 0, 0, 0)]:
        arm_obj.pose.bones["chest"].location = (0, fwd, z)
        arm_obj.pose.bones["chest"].keyframe_insert(data_path="location", frame=f)
        arm_obj.pose.bones["jaw"].rotation_euler = (jaw_angle, 0, 0)
        arm_obj.pose.bones["jaw"].keyframe_insert(data_path="rotation_euler", frame=f)
    add_nla_track(arm_obj, act_attack)
    
    # Action 4: Hound_Death (35 frames)
    act_death = bpy.data.actions.new("Hound_Death")
    arm_obj.animation_data.action = act_death
    for f, z_drop, pitch in [(1, 0, 0), (14, -0.3, 0.3), (35, -0.55, 0.5)]:
        arm_obj.pose.bones["pelvis"].location = (0, 0, z_drop)
        arm_obj.pose.bones["pelvis"].rotation_euler = (pitch, 0, 0.3)
        arm_obj.pose.bones["pelvis"].keyframe_insert(data_path="location", frame=f)
        arm_obj.pose.bones["pelvis"].keyframe_insert(data_path="rotation_euler", frame=f)
    add_nla_track(arm_obj, act_death)
    
    out_path = os.path.join(OUTPUT_DIR, "SK_Hound.fbx")
    export_fbx(out_path, [arm_obj, mesh_obj])


# ==============================================================================
# 2. DEATHMOTH (Aerial Insectoid Flier)
# ==============================================================================
def build_deathmoth():
    print("=== Building Deathmoth Rig & Animations ===")
    reset_scene()
    
    # Procedural Mesh for Deathmoth
    mesh = bpy.data.meshes.new("SKM_Deathmoth")
    bm = bmesh.new()
    
    # Thorax
    bmesh.ops.create_cube(bm, size=0.4)
    # Scale thorax
    for v in bm.verts:
        v.co.y *= 1.4
        v.co.z *= 0.9
        v.co.z += 1.2
    
    # Head & Eyes
    head_bm = bmesh.new()
    bmesh.ops.create_uvsphere(head_bm, u_segments=12, v_segments=8, radius=0.18)
    for v in head_bm.verts:
        v.co.y += 0.35
        v.co.z += 1.25
    head_bm.to_mesh(mesh)
    head_bm.free()
    
    # Abdomen (3 segments)
    for seg, y_off in enumerate([-0.35, -0.65, -0.95]):
        seg_bm = bmesh.new()
        bmesh.ops.create_uvsphere(seg_bm, u_segments=10, v_segments=6, radius=0.18 - seg * 0.03)
        for v in seg_bm.verts:
            v.co.y += y_off
            v.co.z += 1.15 - seg * 0.08
        seg_bm.to_mesh(mesh)
        seg_bm.free()
        
    # Wings (Forewings and Hindwings)
    for side, sign in [("_l", -1), ("_r", 1)]:
        wing_bm = bmesh.new()
        bmesh.ops.create_grid(wing_bm, x_segments=4, y_segments=4, size=0.6)
        for v in wing_bm.verts:
            v.co.x = sign * (abs(v.co.x) * 1.5 + 0.2)
            v.co.y = v.co.y * 0.8 + 0.1
            v.co.z = 1.3
        wing_bm.to_mesh(mesh)
        wing_bm.free()
    
    bm.to_mesh(mesh)
    bm.free()
    
    mesh_obj = bpy.data.objects.new("SKM_Deathmoth", mesh)
    bpy.context.collection.objects.link(mesh_obj)
    
    # Armature
    arm_data = bpy.data.armatures.new("SK_Deathmoth_Skeleton")
    arm_obj = bpy.data.objects.new("Armature_Deathmoth", arm_data)
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
        
    make_bone("root", (0, 0, 0), (0, 0, 0.2))
    make_bone("thorax", (0, 0, 1.2), (0, 0, 1.35), "root")
    make_bone("head", (0, 0.25, 1.25), (0, 0.45, 1.25), "thorax")
    make_bone("AttackSocket", (0, 0.45, 1.25), (0, 0.55, 1.25), "head")
    
    make_bone("abdomen_01", (0, -0.25, 1.15), (0, -0.55, 1.08), "thorax")
    make_bone("abdomen_02", (0, -0.55, 1.08), (0, -0.85, 1.00), "abdomen_01")
    
    make_bone("wing_fore_l", (-0.15, 0.1, 1.3), (-0.95, 0.0, 1.35), "thorax")
    make_bone("wing_fore_r", (0.15, 0.1, 1.3), (0.95, 0.0, 1.35), "thorax")
    make_bone("wing_hind_l", (-0.12, -0.15, 1.25), (-0.65, -0.3, 1.25), "thorax")
    make_bone("wing_hind_r", (0.12, -0.15, 1.25), (0.65, -0.3, 1.25), "thorax")
    
    bpy.ops.object.mode_set(mode='OBJECT')
    
    # Auto Skin
    bpy.ops.object.select_all(action='DESELECT')
    mesh_obj.select_set(True)
    arm_obj.select_set(True)
    bpy.context.view_layer.objects.active = arm_obj
    bpy.ops.object.parent_set(type='ARMATURE_AUTO')
    
    arm_obj.animation_data_create()
    
    # Action 1: Deathmoth_Idle (30 frames, 1s hover)
    act_idle = bpy.data.actions.new("Deathmoth_Idle")
    arm_obj.animation_data.action = act_idle
    for f in range(1, 31):
        phase = (f - 1) / 30.0 * math.pi * 8.0 # 4 flap cycles per second
        flap = math.sin(phase) * 0.6
        arm_obj.pose.bones["wing_fore_l"].rotation_euler = (0, flap, 0)
        arm_obj.pose.bones["wing_fore_l"].keyframe_insert(data_path="rotation_euler", frame=f)
        arm_obj.pose.bones["wing_fore_r"].rotation_euler = (0, -flap, 0)
        arm_obj.pose.bones["wing_fore_r"].keyframe_insert(data_path="rotation_euler", frame=f)
        bob = math.sin((f - 1) / 30.0 * math.pi * 2.0) * 0.05
        arm_obj.pose.bones["thorax"].location = (0, 0, bob)
        arm_obj.pose.bones["thorax"].keyframe_insert(data_path="location", frame=f)
    add_nla_track(arm_obj, act_idle)
    
    # Action 2: Deathmoth_Fly (20 frames)
    act_fly = bpy.data.actions.new("Deathmoth_Fly")
    arm_obj.animation_data.action = act_fly
    for f in range(1, 21):
        phase = (f - 1) / 20.0 * math.pi * 6.0
        flap = math.sin(phase) * 0.8
        arm_obj.pose.bones["wing_fore_l"].rotation_euler = (0.2, flap, -0.2)
        arm_obj.pose.bones["wing_fore_l"].keyframe_insert(data_path="rotation_euler", frame=f)
        arm_obj.pose.bones["wing_fore_r"].rotation_euler = (0.2, -flap, 0.2)
        arm_obj.pose.bones["wing_fore_r"].keyframe_insert(data_path="rotation_euler", frame=f)
    add_nla_track(arm_obj, act_fly)
    
    # Action 3: Deathmoth_Attack (25 frames, dive lunge)
    act_attack = bpy.data.actions.new("Deathmoth_Attack")
    arm_obj.animation_data.action = act_attack
    for f, fwd, z in [(1, 0, 0), (10, 0.5, -0.3), (16, 0.3, 0.0), (25, 0, 0)]:
        arm_obj.pose.bones["thorax"].location = (0, fwd, z)
        arm_obj.pose.bones["thorax"].keyframe_insert(data_path="location", frame=f)
    add_nla_track(arm_obj, act_attack)
    
    # Action 4: Deathmoth_Death (40 frames, tumble drop)
    act_death = bpy.data.actions.new("Deathmoth_Death")
    arm_obj.animation_data.action = act_death
    for f, z, roll in [(1, 0, 0), (20, -0.5, 1.5), (40, -1.2, 3.14)]:
        arm_obj.pose.bones["thorax"].location = (0, 0, z)
        arm_obj.pose.bones["thorax"].rotation_euler = (0, roll, 0)
        arm_obj.pose.bones["thorax"].keyframe_insert(data_path="location", frame=f)
        arm_obj.pose.bones["thorax"].keyframe_insert(data_path="rotation_euler", frame=f)
    add_nla_track(arm_obj, act_death)
    
    out_path = os.path.join(OUTPUT_DIR, "SK_Deathmoth.fbx")
    export_fbx(out_path, [arm_obj, mesh_obj])


# ==============================================================================
# 3. CLUMP (Amorphous Multi-limb Ambush Mass)
# ==============================================================================
def build_clump():
    print("=== Building Clump Rig & Animations ===")
    reset_scene()
    
    mesh = bpy.data.meshes.new("SKM_Clump")
    bm = bmesh.new()
    
    # Mound base
    bmesh.ops.create_uvsphere(bm, u_segments=16, v_segments=10, radius=0.9)
    for v in bm.verts:
        v.co.z *= 0.35
        v.co.z += 0.25
        v.co.x *= 1.2
        v.co.y *= 1.2
        
    # Appendages (8 limbs sticking outwards)
    for i in range(8):
        angle = (i / 8.0) * math.pi * 2.0
        arm_bm = bmesh.new()
        bmesh.ops.create_cone(arm_bm, cap_ends=True, cap_tris=False, segments=8, radius1=0.12, radius2=0.04, depth=0.8)
        cos_a = math.cos(angle)
        sin_a = math.sin(angle)
        for v in arm_bm.verts:
            # Rotate along radial angle
            rx = v.co.x * cos_a - v.co.z * sin_a
            ry = v.co.x * sin_a + v.co.z * cos_a
            v.co.x = rx + cos_a * 0.9
            v.co.y = ry + sin_a * 0.9
            v.co.z = 0.2
        arm_bm.to_mesh(mesh)
        arm_bm.free()
        
    bm.to_mesh(mesh)
    bm.free()
    
    mesh_obj = bpy.data.objects.new("SKM_Clump", mesh)
    bpy.context.collection.objects.link(mesh_obj)
    
    # Armature
    arm_data = bpy.data.armatures.new("SK_Clump_Skeleton")
    arm_obj = bpy.data.objects.new("Armature_Clump", arm_data)
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
        
    make_bone("root", (0, 0, 0), (0, 0, 0.1))
    make_bone("mound_core", (0, 0, 0.15), (0, 0, 0.45), "root")
    make_bone("AttackSocket", (0, 0.6, 0.35), (0, 0.9, 0.35), "mound_core")
    
    for i in range(8):
        angle = (i / 8.0) * math.pi * 2.0
        ca = math.cos(angle)
        sa = math.sin(angle)
        make_bone(f"limb_{i+1}_base", (ca * 0.3, sa * 0.3, 0.25), (ca * 0.7, sa * 0.7, 0.25), "mound_core")
        make_bone(f"limb_{i+1}_tip", (ca * 0.7, sa * 0.7, 0.25), (ca * 1.2, sa * 1.2, 0.1), f"limb_{i+1}_base")
        
    bpy.ops.object.mode_set(mode='OBJECT')
    
    # Auto Skin
    bpy.ops.object.select_all(action='DESELECT')
    mesh_obj.select_set(True)
    arm_obj.select_set(True)
    bpy.context.view_layer.objects.active = arm_obj
    bpy.ops.object.parent_set(type='ARMATURE_AUTO')
    
    arm_obj.animation_data_create()
    
    # Action 1: Clump_Idle (60 frames, undulating mound)
    act_idle = bpy.data.actions.new("Clump_Idle")
    arm_obj.animation_data.action = act_idle
    for f in range(1, 61):
        t = (f - 1) / 60.0 * math.pi * 2.0
        z_scale = 1.0 + math.sin(t) * 0.12
        arm_obj.pose.bones["mound_core"].scale = (1.0, 1.0, z_scale)
        arm_obj.pose.bones["mound_core"].keyframe_insert(data_path="scale", frame=f)
    add_nla_track(arm_obj, act_idle)
    
    # Action 2: Clump_Slither (40 frames)
    act_slither = bpy.data.actions.new("Clump_Slither")
    arm_obj.animation_data.action = act_slither
    for f in range(1, 41):
        t = (f - 1) / 40.0 * math.pi * 2.0
        arm_obj.pose.bones["mound_core"].location = (0, math.sin(t) * 0.25, math.cos(t) * 0.05)
        arm_obj.pose.bones["mound_core"].keyframe_insert(data_path="location", frame=f)
    add_nla_track(arm_obj, act_slither)
    
    # Action 3: Clump_Attack (30 frames, eruptive grab)
    act_attack = bpy.data.actions.new("Clump_Attack")
    arm_obj.animation_data.action = act_attack
    for f, fwd, z in [(1, 0, 0), (12, 0.6, 0.35), (20, 0.4, 0.1), (30, 0, 0)]:
        arm_obj.pose.bones["mound_core"].location = (0, fwd, z)
        arm_obj.pose.bones["mound_core"].keyframe_insert(data_path="location", frame=f)
    add_nla_track(arm_obj, act_attack)
    
    # Action 4: Clump_Death (45 frames, collapse flat)
    act_death = bpy.data.actions.new("Clump_Death")
    arm_obj.animation_data.action = act_death
    for f, z_scale in [(1, 1.0), (20, 0.4), (45, 0.15)]:
        arm_obj.pose.bones["mound_core"].scale = (1.3, 1.3, z_scale)
        arm_obj.pose.bones["mound_core"].keyframe_insert(data_path="scale", frame=f)
    add_nla_track(arm_obj, act_death)
    
    out_path = os.path.join(OUTPUT_DIR, "SK_Clump.fbx")
    export_fbx(out_path, [arm_obj, mesh_obj])


# ==============================================================================
# 4. JERRY (Anomalous Psionic Avian)
# ==============================================================================
def build_jerry():
    print("=== Building Jerry Rig & Animations ===")
    reset_scene()
    
    mesh = bpy.data.meshes.new("SKM_Jerry")
    bm = bmesh.new()
    
    # Body (Macaw torso)
    bmesh.ops.create_uvsphere(bm, u_segments=12, v_segments=8, radius=0.22)
    for v in bm.verts:
        v.co.z = v.co.z * 1.3 + 0.35
        v.co.y = v.co.y * 1.1 - 0.05
        
    # Head & Curved Beak
    head_bm = bmesh.new()
    bmesh.ops.create_uvsphere(head_bm, u_segments=10, v_segments=6, radius=0.12)
    for v in head_bm.verts:
        v.co.y += 0.15
        v.co.z += 0.58
    head_bm.to_mesh(mesh)
    head_bm.free()
    
    # Beak
    beak_bm = bmesh.new()
    bmesh.ops.create_cone(beak_bm, cap_ends=True, cap_tris=False, segments=6, radius1=0.06, radius2=0.01, depth=0.14)
    for v in beak_bm.verts:
        v.co.y += 0.28
        v.co.z += 0.54
    beak_bm.to_mesh(mesh)
    beak_bm.free()
    
    # Wings
    for side, sign in [("_l", -1), ("_r", 1)]:
        wing_bm = bmesh.new()
        bmesh.ops.create_cube(wing_bm, size=0.25)
        for v in wing_bm.verts:
            v.co.x = sign * (abs(v.co.x) * 1.8 + 0.15)
            v.co.y *= 0.6
            v.co.z = v.co.z * 0.8 + 0.38
        wing_bm.to_mesh(mesh)
        wing_bm.free()
        
    bm.to_mesh(mesh)
    bm.free()
    
    mesh_obj = bpy.data.objects.new("SKM_Jerry", mesh)
    bpy.context.collection.objects.link(mesh_obj)
    
    # Armature
    arm_data = bpy.data.armatures.new("SK_Jerry_Skeleton")
    arm_obj = bpy.data.objects.new("Armature_Jerry", arm_data)
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
        
    make_bone("root", (0, 0, 0), (0, 0, 0.1))
    make_bone("body", (0, 0, 0.2), (0, 0, 0.45), "root")
    make_bone("neck", (0, 0.08, 0.45), (0, 0.12, 0.54), "body")
    make_bone("head", (0, 0.12, 0.54), (0, 0.22, 0.62), "neck")
    make_bone("AttackSocket", (0, 0.28, 0.55), (0, 0.38, 0.55), "head")
    
    make_bone("wing_l", (-0.15, 0.0, 0.4), (-0.45, -0.05, 0.35), "body")
    make_bone("wing_r", (0.15, 0.0, 0.4), (0.45, -0.05, 0.35), "body")
    make_bone("tail", (0, -0.2, 0.25), (0, -0.45, 0.15), "body")
    
    make_bone("leg_l", (-0.08, 0.0, 0.2), (-0.08, 0.0, 0.0), "body")
    make_bone("leg_r", (0.08, 0.0, 0.2), (0.08, 0.0, 0.0), "body")
    
    bpy.ops.object.mode_set(mode='OBJECT')
    
    # Auto Skin
    bpy.ops.object.select_all(action='DESELECT')
    mesh_obj.select_set(True)
    arm_obj.select_set(True)
    bpy.context.view_layer.objects.active = arm_obj
    bpy.ops.object.parent_set(type='ARMATURE_AUTO')
    
    arm_obj.animation_data_create()
    
    # Action 1: Jerry_Idle (60 frames, perching idle with head twitches)
    act_idle = bpy.data.actions.new("Jerry_Idle")
    arm_obj.animation_data.action = act_idle
    for f, yaw in [(1, 0), (15, 0.5), (25, 0.5), (35, -0.6), (45, -0.6), (60, 0)]:
        arm_obj.pose.bones["head"].rotation_euler = (0, 0, yaw)
        arm_obj.pose.bones["head"].keyframe_insert(data_path="rotation_euler", frame=f)
    add_nla_track(arm_obj, act_idle)
    
    # Action 2: Jerry_Hop (16 frames, avian double hop)
    act_hop = bpy.data.actions.new("Jerry_Hop")
    arm_obj.animation_data.action = act_hop
    for f, z, y in [(1, 0, 0), (6, 0.18, 0.12), (11, 0.0, 0.22), (16, 0, 0.22)]:
        arm_obj.pose.bones["body"].location = (0, y, z)
        arm_obj.pose.bones["body"].keyframe_insert(data_path="location", frame=f)
    add_nla_track(arm_obj, act_hop)
    
    # Action 3: Jerry_Hypnosis (40 frames, psionic channel with wing flare)
    act_hypno = bpy.data.actions.new("Jerry_Hypnosis")
    arm_obj.animation_data.action = act_hypno
    for f, flare in [(1, 0), (10, 0.9), (30, 0.9), (40, 0)]:
        arm_obj.pose.bones["wing_l"].rotation_euler = (0, flare, 0)
        arm_obj.pose.bones["wing_l"].keyframe_insert(data_path="rotation_euler", frame=f)
        arm_obj.pose.bones["wing_r"].rotation_euler = (0, -flare, 0)
        arm_obj.pose.bones["wing_r"].keyframe_insert(data_path="rotation_euler", frame=f)
    add_nla_track(arm_obj, act_hypno)
    
    # Action 4: Jerry_Death (30 frames, topple backward)
    act_death = bpy.data.actions.new("Jerry_Death")
    arm_obj.animation_data.action = act_death
    for f, pitch in [(1, 0), (15, -1.2), (30, -1.57)]:
        arm_obj.pose.bones["body"].rotation_euler = (pitch, 0, 0)
        arm_obj.pose.bones["body"].keyframe_insert(data_path="rotation_euler", frame=f)
    add_nla_track(arm_obj, act_death)
    
    out_path = os.path.join(OUTPUT_DIR, "SK_Jerry.fbx")
    export_fbx(out_path, [arm_obj, mesh_obj])


if __name__ == "__main__":
    print("Starting Blender procedural Bestiary generation...")
    build_hound()
    build_deathmoth()
    build_clump()
    build_jerry()
    print("All 4 Bestiary skeletal meshes successfully generated!")
