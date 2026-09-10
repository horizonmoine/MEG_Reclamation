import bpy
import bmesh
import math
import os
import sys

OUTPUT_DIR = r"F:\MEG_Reclamation\RawAssets\FBX\Bestiary"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    for c in list(bpy.data.collections):
        bpy.data.collections.remove(c)
    col = bpy.data.collections.new("ExportCollection")
    bpy.context.scene.collection.children.link(col)
    bpy.context.view_layer.active_layer_collection = bpy.context.view_layer.layer_collection.children["ExportCollection"]
    return col

def export_fbx(filepath, objects):
    bpy.ops.object.select_all(action='DESELECT')
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    
    print(f"Exporting FBX: {filepath}")
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

def create_pbr_material(name, base_color, roughness=0.5, metallic=0.0, emission_color=None, emission_strength=0.0):
    mat = bpy.data.materials.new(name=name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    if bsdf:
        bsdf.inputs['Base Color'].default_value = base_color
        bsdf.inputs['Roughness'].default_value = roughness
        bsdf.inputs['Metallic'].default_value = metallic
        if emission_color and 'Emission Color' in bsdf.inputs:
            bsdf.inputs['Emission Color'].default_value = emission_color
            if 'Emission Strength' in bsdf.inputs:
                bsdf.inputs['Emission Strength'].default_value = emission_strength
    return mat

# ==============================================================================
# 1. SMILER (Luminescent teeth and eyes floating in dark shadow silhouette)
# ==============================================================================
def build_smiler():
    print("=== Building Smiler Rig & Animations ===")
    reset_scene()
    
    mat_dark = create_pbr_material("Mat_Smiler_Darkness", (0.01, 0.01, 0.01, 1.0), roughness=0.95, metallic=0.0)
    mat_glow = create_pbr_material("Mat_Smiler_Glow", (0.9, 1.0, 0.9, 1.0), roughness=0.1, metallic=0.0, emission_color=(0.95, 1.0, 0.95, 1.0), emission_strength=8.0)
    mat_teeth = create_pbr_material("Mat_Smiler_Teeth", (1.0, 1.0, 0.85, 1.0), roughness=0.2, metallic=0.0, emission_color=(1.0, 1.0, 0.85, 1.0), emission_strength=5.0)

    # Torso/silhouette
    bpy.ops.mesh.primitive_cylinder_add(radius=0.35, depth=1.2, location=(0, 0, 1.0))
    body = bpy.context.active_object
    body.name = "SKM_Smiler"
    body.scale = (0.8, 0.4, 1.0)
    bpy.ops.object.transform_apply(scale=True)
    
    parts = [body]
    
    # Head / Skull shadow
    bpy.ops.mesh.primitive_uv_sphere_add(radius=0.32, location=(0, 0, 1.75))
    head = bpy.context.active_object
    parts.append(head)
    
    # Glowing Eyes (two piercing white/green light spheres)
    for x_side in [-0.14, 0.14]:
        bpy.ops.mesh.primitive_uv_sphere_add(radius=0.045, location=(x_side, 0.28, 1.85))
        eye = bpy.context.active_object
        eye.data.materials.append(mat_glow)
        parts.append(eye)
        
    # Iconic Glowing Smile Arc (16 sharp teeth arranged along a crescent)
    for i in range(16):
        t = (i - 7.5) / 7.5
        angle = t * 1.1
        x = math.sin(angle) * 0.22
        y = math.cos(angle) * 0.26
        z_curve = 1.62 - (1.0 - math.cos(angle)) * 0.06
        
        z_offset = 0.025 if (i % 2 == 0) else -0.025
        bpy.ops.mesh.primitive_cone_add(radius1=0.016, depth=0.05, location=(x, y, z_curve + z_offset))
        tooth = bpy.context.active_object
        tooth.rotation_euler = (math.radians(-90) if (i % 2 == 0) else math.radians(90), 0, angle * 0.5)
        bpy.ops.object.transform_apply(rotation=True)
        tooth.data.materials.append(mat_teeth)
        parts.append(tooth)
        
    body.data.materials.append(mat_dark)
    
    bpy.ops.object.select_all(action='DESELECT')
    for p in parts:
        p.select_set(True)
    bpy.context.view_layer.objects.active = body
    bpy.ops.object.join()
    
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.normals_make_consistent(inside=False)
    bpy.ops.uv.smart_project(angle_limit=math.radians(66.0), island_margin=0.02)
    bpy.ops.object.mode_set(mode='OBJECT')
    
    bpy.ops.object.armature_add(location=(0, 0, 0))
    arm_obj = bpy.context.active_object
    arm_obj.name = "SK_Smiler_Skeleton"
    arm_data = arm_obj.data
    arm_data.name = "SK_Smiler_Armature"
    
    bpy.ops.object.mode_set(mode='EDIT')
    eb = arm_data.edit_bones
    root = eb[0]
    root.name = "root"
    root.head = (0, 0, 0)
    root.tail = (0, 0, 0.4)
    
    spine = eb.new("spine")
    spine.head = (0, 0, 0.4)
    spine.tail = (0, 0, 1.4)
    spine.parent = root
    
    head_bone = eb.new("head")
    head_bone.head = (0, 0, 1.4)
    head_bone.tail = (0, 0, 2.0)
    head_bone.parent = spine
    bpy.ops.object.mode_set(mode='OBJECT')
    
    bpy.ops.object.select_all(action='DESELECT')
    body.select_set(True)
    arm_obj.select_set(True)
    bpy.context.view_layer.objects.active = arm_obj
    bpy.ops.object.parent_set(type='ARMATURE_AUTO')
    
    arm_obj.animation_data_create()
    
    act_idle = bpy.data.actions.new("Smiler_Idle")
    arm_obj.animation_data.action = act_idle
    for f in range(1, 61):
        bob = math.sin((f - 1) / 60.0 * math.pi * 2.0) * 0.08
        arm_obj.pose.bones["head"].location = (0, 0, bob)
        arm_obj.pose.bones["head"].keyframe_insert(data_path="location", frame=f)
    add_nla_track(arm_obj, act_idle)
    
    act_charge = bpy.data.actions.new("Smiler_Charge")
    arm_obj.animation_data.action = act_charge
    for f, y, tilt in [(1, 0, 0), (10, 0.3, 0.25), (20, 0.5, 0.3), (30, 0, 0)]:
        arm_obj.pose.bones["head"].location = (0, y, 0)
        arm_obj.pose.bones["head"].rotation_euler = (tilt, 0, 0)
        arm_obj.pose.bones["head"].keyframe_insert(data_path="location", frame=f)
        arm_obj.pose.bones["head"].keyframe_insert(data_path="rotation_euler", frame=f)
    add_nla_track(arm_obj, act_charge)
    
    out_path = os.path.join(OUTPUT_DIR, "SK_Smiler.fbx")
    export_fbx(out_path, [arm_obj, body])

# ==============================================================================
# 2. PARTYGOER (Tall yellow humanoid with carved smile and party balloon)
# ==============================================================================
def build_partygoer():
    print("=== Building Partygoer Rig & Animations ===")
    reset_scene()
    
    mat_skin = create_pbr_material("Mat_Partygoer_Skin", (0.92, 0.78, 0.18, 1.0), roughness=0.6, metallic=0.05)
    mat_smile = create_pbr_material("Mat_Partygoer_Smile", (0.85, 0.05, 0.05, 1.0), roughness=0.3, metallic=0.0)
    mat_balloon = create_pbr_material("Mat_Balloon_LatexRed", (0.9, 0.08, 0.08, 1.0), roughness=0.15, metallic=0.0)
    
    bpy.ops.mesh.primitive_cylinder_add(radius=0.22, depth=0.8, location=(0, 0, 1.3))
    torso = bpy.context.active_object
    torso.name = "SKM_Partygoer"
    torso.scale = (0.9, 0.6, 1.0)
    bpy.ops.object.transform_apply(scale=True)
    parts = [torso]
    
    bpy.ops.mesh.primitive_uv_sphere_add(radius=0.20, location=(0, 0, 1.85))
    head = bpy.context.active_object
    head.scale = (0.85, 0.9, 1.1)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(head)
    
    for x in [-0.07, 0.07]:
        bpy.ops.mesh.primitive_cube_add(size=0.025, location=(x, 0.18, 1.90))
        eye = bpy.context.active_object
        eye.data.materials.append(mat_smile)
        parts.append(eye)
        
    for i in range(9):
        t = (i - 4) / 4.0
        angle = t * 0.9
        x = math.sin(angle) * 0.09
        y = 0.17 + math.cos(angle) * 0.02
        z = 1.78 - (1.0 - math.cos(angle)) * 0.03
        bpy.ops.mesh.primitive_cube_add(size=0.018, location=(x, y, z))
        dot = bpy.context.active_object
        dot.data.materials.append(mat_smile)
        parts.append(dot)
        
    for side, x in [(-1, -0.32), (1, 0.32)]:
        bpy.ops.mesh.primitive_cylinder_add(radius=0.05, depth=0.9, location=(x, 0, 1.15))
        arm = bpy.context.active_object
        parts.append(arm)
        
    for side, x in [(-1, -0.14), (1, 0.14)]:
        bpy.ops.mesh.primitive_cylinder_add(radius=0.07, depth=0.9, location=(x, 0, 0.45))
        leg = bpy.context.active_object
        parts.append(leg)
        
    bpy.ops.mesh.primitive_uv_sphere_add(radius=0.18, location=(0.42, 0.1, 2.25))
    balloon = bpy.context.active_object
    balloon.scale = (1.0, 1.0, 1.25)
    bpy.ops.object.transform_apply(scale=True)
    balloon.data.materials.append(mat_balloon)
    parts.append(balloon)
    
    torso.data.materials.append(mat_skin)
    
    bpy.ops.object.select_all(action='DESELECT')
    for p in parts:
        p.select_set(True)
    bpy.context.view_layer.objects.active = torso
    bpy.ops.object.join()
    
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.normals_make_consistent(inside=False)
    bpy.ops.uv.smart_project(angle_limit=math.radians(66.0), island_margin=0.02)
    bpy.ops.object.mode_set(mode='OBJECT')
    
    bpy.ops.object.armature_add(location=(0, 0, 0))
    arm_obj = bpy.context.active_object
    arm_obj.name = "SK_Partygoer_Skeleton"
    arm_data = arm_obj.data
    arm_data.name = "SK_Partygoer_Armature"
    
    bpy.ops.object.mode_set(mode='EDIT')
    eb = arm_data.edit_bones
    root = eb[0]
    root.name = "root"
    root.head = (0, 0, 0)
    root.tail = (0, 0, 0.9)
    
    spine = eb.new("spine")
    spine.head = (0, 0, 0.9)
    spine.tail = (0, 0, 1.7)
    spine.parent = root
    
    head_bone = eb.new("head")
    head_bone.head = (0, 0, 1.7)
    head_bone.tail = (0, 0, 2.1)
    head_bone.parent = spine
    bpy.ops.object.mode_set(mode='OBJECT')
    
    bpy.ops.object.select_all(action='DESELECT')
    torso.select_set(True)
    arm_obj.select_set(True)
    bpy.context.view_layer.objects.active = arm_obj
    bpy.ops.object.parent_set(type='ARMATURE_AUTO')
    
    arm_obj.animation_data_create()
    act_idle = bpy.data.actions.new("Partygoer_Idle")
    arm_obj.animation_data.action = act_idle
    for f in range(1, 61):
        sway = math.sin((f - 1) / 60.0 * math.pi * 2.0) * 0.04
        arm_obj.pose.bones["spine"].rotation_euler = (0, sway, 0)
        arm_obj.pose.bones["spine"].keyframe_insert(data_path="rotation_euler", frame=f)
    add_nla_track(arm_obj, act_idle)
    
    out_path = os.path.join(OUTPUT_DIR, "SK_Partygoer.fbx")
    export_fbx(out_path, [arm_obj, torso])

# ==============================================================================
# 3. SKIN-STEALER (Gaunt hunched creature with mottled hanging skin and claws)
# ==============================================================================
def build_skin_stealer():
    print("=== Building Skin-Stealer Rig & Animations ===")
    reset_scene()
    
    mat_flesh = create_pbr_material("Mat_SkinStealer_Flesh", (0.55, 0.48, 0.42, 1.0), roughness=0.7, metallic=0.0)
    mat_claws = create_pbr_material("Mat_SkinStealer_Claws", (0.15, 0.12, 0.10, 1.0), roughness=0.3, metallic=0.2)
    
    bpy.ops.mesh.primitive_cylinder_add(radius=0.26, depth=0.9, location=(0, 0.15, 1.1))
    torso = bpy.context.active_object
    torso.name = "SKM_SkinStealer"
    torso.rotation_euler = (math.radians(25), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    parts = [torso]
    
    bpy.ops.mesh.primitive_uv_sphere_add(radius=0.22, location=(0, 0.45, 1.5))
    head = bpy.context.active_object
    parts.append(head)
    
    for side, x in [(-1, -0.38), (1, 0.38)]:
        bpy.ops.mesh.primitive_cylinder_add(radius=0.06, depth=1.1, location=(x, 0.25, 0.9))
        arm = bpy.context.active_object
        parts.append(arm)
        for c in [-0.03, 0.0, 0.03]:
            bpy.ops.mesh.primitive_cone_add(radius1=0.015, depth=0.12, location=(x + c, 0.25, 0.3))
            claw = bpy.context.active_object
            claw.data.materials.append(mat_claws)
            parts.append(claw)
            
    for side, x in [(-1, -0.18), (1, 0.18)]:
        bpy.ops.mesh.primitive_cylinder_add(radius=0.08, depth=0.85, location=(x, -0.1, 0.42))
        leg = bpy.context.active_object
        parts.append(leg)
        
    torso.data.materials.append(mat_flesh)
    
    bpy.ops.object.select_all(action='DESELECT')
    for p in parts:
        p.select_set(True)
    bpy.context.view_layer.objects.active = torso
    bpy.ops.object.join()
    
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.normals_make_consistent(inside=False)
    bpy.ops.uv.smart_project(angle_limit=math.radians(66.0), island_margin=0.02)
    bpy.ops.object.mode_set(mode='OBJECT')
    
    bpy.ops.object.armature_add(location=(0, 0, 0))
    arm_obj = bpy.context.active_object
    arm_obj.name = "SK_SkinStealer_Skeleton"
    arm_data = arm_obj.data
    arm_data.name = "SK_SkinStealer_Armature"
    
    bpy.ops.object.mode_set(mode='EDIT')
    eb = arm_data.edit_bones
    root = eb[0]
    root.name = "root"
    root.head = (0, 0, 0)
    root.tail = (0, 0, 0.8)
    
    spine = eb.new("spine")
    spine.head = (0, 0, 0.8)
    spine.tail = (0, 0.3, 1.4)
    spine.parent = root
    
    head_bone = eb.new("head")
    head_bone.head = (0, 0.3, 1.4)
    head_bone.tail = (0, 0.5, 1.6)
    head_bone.parent = spine
    bpy.ops.object.mode_set(mode='OBJECT')
    
    bpy.ops.object.select_all(action='DESELECT')
    torso.select_set(True)
    arm_obj.select_set(True)
    bpy.context.view_layer.objects.active = arm_obj
    bpy.ops.object.parent_set(type='ARMATURE_AUTO')
    
    arm_obj.animation_data_create()
    act_idle = bpy.data.actions.new("SkinStealer_Idle")
    arm_obj.animation_data.action = act_idle
    for f in range(1, 61):
        twitch = math.sin((f - 1) / 60.0 * math.pi * 4.0) * 0.05
        arm_obj.pose.bones["head"].rotation_euler = (twitch, 0, twitch * 0.5)
        arm_obj.pose.bones["head"].keyframe_insert(data_path="rotation_euler", frame=f)
    add_nla_track(arm_obj, act_idle)
    
    out_path = os.path.join(OUTPUT_DIR, "SK_SkinStealer.fbx")
    export_fbx(out_path, [arm_obj, torso])

if __name__ == "__main__":
    print("=== Generating Smiler, Partygoer, and Skin-Stealer ===")
    build_smiler()
    build_partygoer()
    build_skin_stealer()
    print("=== Complete! ===")
