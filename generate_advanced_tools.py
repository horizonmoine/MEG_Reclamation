import bpy
import bmesh
import math
import os

OUTPUT_DIR = r"F:\MEG_Reclamation\RawAssets\FBX\Tools"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    for collection in [bpy.data.objects, bpy.data.meshes, bpy.data.materials]:
        for item in list(collection):
            collection.remove(item, do_unlink=True)

def create_pbr_material(name, base_color, metallic=0.0, roughness=0.5, emission_color=(0,0,0,1), emission_strength=0.0):
    mat = bpy.data.materials.new(name=name)
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    bsdf = nodes.get("Principled BSDF")
    if bsdf:
        bsdf.inputs['Base Color'].default_value = (*base_color, 1.0)
        bsdf.inputs['Metallic'].default_value = metallic
        bsdf.inputs['Roughness'].default_value = roughness
        if emission_strength > 0.0:
            bsdf.inputs['Emission Color'].default_value = emission_color
            bsdf.inputs['Emission Strength'].default_value = emission_strength
    return mat

def apply_modifiers_and_uv(obj):
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.smart_project(angle_limit=math.radians(66.0), island_margin=0.02)
    bpy.ops.object.mode_set(mode='OBJECT')

def export_fbx(obj, filename):
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    filepath = os.path.join(OUTPUT_DIR, filename)
    bpy.ops.export_scene.fbx(
        filepath=filepath,
        use_selection=True,
        global_scale=1.0,
        apply_unit_scale=True,
        apply_scale_options='FBX_SCALE_ALL',
        mesh_smooth_type='FACE',
        add_leaf_bones=False
    )
    print(f"Exported tool FBX: {filepath}")

# 1. SM_Lidar_Scanner
reset_scene()
mat_casing = create_pbr_material("Mat_Lidar_Polymer", (0.12, 0.13, 0.14), metallic=0.2, roughness=0.6)
mat_lens = create_pbr_material("Mat_Lidar_Optics", (0.05, 0.05, 0.08), metallic=0.9, roughness=0.1)
mat_screen = create_pbr_material("Mat_Lidar_Screen", (0.0, 0.8, 0.4), metallic=0.1, roughness=0.2, emission_color=(0,1,0.5,1), emission_strength=3.0)

# Body
bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0))
lidar_body = bpy.context.active_object
lidar_body.name = "SM_Lidar_Scanner"
lidar_body.scale = (0.08, 0.14, 0.10)
lidar_body.data.materials.append(mat_casing)

# Turret
bpy.ops.mesh.primitive_cylinder_add(radius=0.04, depth=0.05, location=(0, 0.03, 0.075))
lidar_turret = bpy.context.active_object
lidar_turret.data.materials.append(mat_lens)

# Screen
bpy.ops.mesh.primitive_plane_add(size=1.0, location=(0, -0.071, 0.02), rotation=(math.radians(90), 0, 0))
lidar_screen = bpy.context.active_object
lidar_screen.scale = (0.05, 0.04, 1.0)
lidar_screen.data.materials.append(mat_screen)

# Join
bpy.ops.object.select_all(action='SELECT')
bpy.context.view_layer.objects.active = lidar_body
bpy.ops.object.join()
apply_modifiers_and_uv(lidar_body)
export_fbx(lidar_body, "SM_Lidar_Scanner.fbx")

# 2. SM_Signal_Analyzer
reset_scene()
mat_box = create_pbr_material("Mat_Signal_Box", (0.75, 0.55, 0.08), metallic=0.1, roughness=0.7)
mat_antenna = create_pbr_material("Mat_Signal_Metal", (0.8, 0.82, 0.85), metallic=0.95, roughness=0.2)
mat_scope = create_pbr_material("Mat_Signal_Phosphor", (0.1, 0.8, 0.9), metallic=0.1, roughness=0.1, emission_color=(0.1, 0.9, 1.0, 1), emission_strength=4.0)

bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0))
sig_body = bpy.context.active_object
sig_body.name = "SM_Signal_Analyzer"
sig_body.scale = (0.10, 0.15, 0.06)
sig_body.data.materials.append(mat_box)

# Antenna
bpy.ops.mesh.primitive_cylinder_add(radius=0.006, depth=0.35, location=(-0.04, 0.06, 0.18))
sig_antenna = bpy.context.active_object
sig_antenna.data.materials.append(mat_antenna)

# Scope screen
bpy.ops.mesh.primitive_cylinder_add(radius=0.035, depth=0.01, location=(0, -0.01, 0.032))
sig_scope = bpy.context.active_object
sig_scope.data.materials.append(mat_scope)

bpy.ops.object.select_all(action='SELECT')
bpy.context.view_layer.objects.active = sig_body
bpy.ops.object.join()
apply_modifiers_and_uv(sig_body)
export_fbx(sig_body, "SM_Signal_Analyzer.fbx")

# 3. SM_Reality_Anchor
reset_scene()
mat_brass = create_pbr_material("Mat_Anchor_Brass", (0.85, 0.65, 0.22), metallic=0.85, roughness=0.3)
mat_steel = create_pbr_material("Mat_Anchor_Steel", (0.25, 0.26, 0.28), metallic=0.9, roughness=0.4)
mat_core = create_pbr_material("Mat_Anchor_Core", (0.8, 0.1, 0.9), metallic=0.2, roughness=0.1, emission_color=(0.9, 0.2, 1.0, 1), emission_strength=5.0)

# Shaft
bpy.ops.mesh.primitive_cylinder_add(radius=0.025, depth=0.40, location=(0, 0, 0))
anchor_body = bpy.context.active_object
anchor_body.name = "SM_Reality_Anchor"
anchor_body.data.materials.append(mat_brass)

# Gimbal Ring
bpy.ops.mesh.primitive_torus_add(major_radius=0.08, minor_radius=0.012, location=(0, 0, 0.05))
anchor_ring = bpy.context.active_object
anchor_ring.data.materials.append(mat_steel)

# Crystal core
bpy.ops.mesh.primitive_ico_sphere_add(radius=0.035, subdivisions=2, location=(0, 0, 0.05))
anchor_crystal = bpy.context.active_object
anchor_crystal.data.materials.append(mat_core)

bpy.ops.object.select_all(action='SELECT')
bpy.context.view_layer.objects.active = anchor_body
bpy.ops.object.join()
apply_modifiers_and_uv(anchor_body)
export_fbx(anchor_body, "SM_Reality_Anchor.fbx")

# 4. SM_Sonic_Microwave
reset_scene()
mat_sonic_gun = create_pbr_material("Mat_Sonic_Body", (0.2, 0.22, 0.25), metallic=0.8, roughness=0.4)
mat_dish = create_pbr_material("Mat_Sonic_Dish", (0.7, 0.5, 0.15), metallic=0.9, roughness=0.25)
mat_coil = create_pbr_material("Mat_Sonic_Coil", (0.1, 0.5, 0.9), metallic=0.5, roughness=0.3, emission_color=(0.2, 0.6, 1.0, 1), emission_strength=3.5)

# Receiver / Gun Handle
bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, -0.06, -0.04))
sonic_body = bpy.context.active_object
sonic_body.name = "SM_Sonic_Microwave"
sonic_body.scale = (0.05, 0.10, 0.14)
sonic_body.data.materials.append(mat_sonic_gun)

# Conical horn
bpy.ops.mesh.primitive_cone_add(radius1=0.10, radius2=0.03, depth=0.15, location=(0, 0.08, 0.02), rotation=(math.radians(-90), 0, 0))
sonic_dish = bpy.context.active_object
sonic_dish.data.materials.append(mat_dish)

# Central resonant emitter
bpy.ops.mesh.primitive_cylinder_add(radius=0.015, depth=0.12, location=(0, 0.08, 0.02), rotation=(math.radians(-90), 0, 0))
sonic_coil = bpy.context.active_object
sonic_coil.data.materials.append(mat_coil)

bpy.ops.object.select_all(action='SELECT')
bpy.context.view_layer.objects.active = sonic_body
bpy.ops.object.join()
apply_modifiers_and_uv(sonic_body)
export_fbx(sonic_body, "SM_Sonic_Microwave.fbx")

# 5. SM_Adrenaline_Injector
reset_scene()
mat_syringe_body = create_pbr_material("Mat_Injector_Polymer", (0.18, 0.20, 0.22), metallic=0.1, roughness=0.5)
mat_fluid = create_pbr_material("Mat_Injector_Fluid", (0.9, 0.6, 0.1), metallic=0.1, roughness=0.1, emission_color=(1.0, 0.7, 0.1, 1), emission_strength=2.5)
mat_needle = create_pbr_material("Mat_Injector_Chrome", (0.9, 0.92, 0.95), metallic=0.98, roughness=0.1)

# Barrel
bpy.ops.mesh.primitive_cylinder_add(radius=0.018, depth=0.18, location=(0, 0, 0))
inj_body = bpy.context.active_object
inj_body.name = "SM_Adrenaline_Injector"
inj_body.data.materials.append(mat_syringe_body)

# Glowing fluid vial
bpy.ops.mesh.primitive_cylinder_add(radius=0.014, depth=0.08, location=(0, 0, 0.01))
inj_fluid = bpy.context.active_object
inj_fluid.data.materials.append(mat_fluid)

# Needle
bpy.ops.mesh.primitive_cylinder_add(radius=0.003, depth=0.05, location=(0, 0, -0.11))
inj_needle = bpy.context.active_object
inj_needle.data.materials.append(mat_needle)

bpy.ops.object.select_all(action='SELECT')
bpy.context.view_layer.objects.active = inj_body
bpy.ops.object.join()
apply_modifiers_and_uv(inj_body)
export_fbx(inj_body, "SM_Adrenaline_Injector.fbx")

print("=== ALL 5 ADVANCED SCAVENGER TOOLS GENERATED SUCCESSFULLY ===")
