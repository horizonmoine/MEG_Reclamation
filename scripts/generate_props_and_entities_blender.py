"""
Procedural 3D Mesh Generator for M.E.G. : Reclamation
Compatible with Blender 5.2.1 LTS
Models and exports:
1. SM_Harmonic_Resonator (M.E.G. No-clip tool)
2. SM_Trilight_Flashlight (3-mode tactical flashlight)
3. SK_Hydrolitis (Fluid contaminated mono-ethylene glycol creature)
"""

import bpy
import bmesh
import math
import os

PROPS_DIR = r"F:\MEG_Reclamation\RawAssets\FBX\Props"
BESTIARY_DIR = r"F:\MEG_Reclamation\RawAssets\FBX\Bestiary"
os.makedirs(PROPS_DIR, exist_ok=True)
os.makedirs(BESTIARY_DIR, exist_ok=True)

def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    for c in list(bpy.data.collections):
        bpy.data.collections.remove(c)
    col = bpy.data.collections.new("ExportCollection")
    bpy.context.scene.collection.children.link(col)
    bpy.context.view_layer.active_layer_collection = bpy.context.view_layer.layer_collection.children["ExportCollection"]
    return col

def export_static_mesh(filepath, obj):
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    
    print(f"Exporting Static Mesh FBX: {filepath}")
    bpy.ops.export_scene.fbx(
        filepath=filepath,
        use_selection=True,
        global_scale=1.0,
        apply_unit_scale=True,
        apply_scale_options='FBX_SCALE_NONE',
        bake_space_transform=False,
        object_types={'MESH'},
        use_mesh_modifiers=True,
        mesh_smooth_type='FACE'
    )
    print(f"Exported: {filepath} ({os.path.getsize(filepath)} bytes)")

# ==============================================================================
# 1. HARMONIC RESONATOR (M.E.G. Maintenance Tool)
# ==============================================================================
def build_harmonic_resonator():
    print("=== Building Harmonic Resonator ===")
    reset_scene()
    
    # Base Box (20cm x 30cm x 12cm)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.06))
    base = bpy.context.active_object
    base.name = "SM_Harmonic_Resonator"
    base.scale = (0.20, 0.30, 0.12)
    bpy.ops.object.transform_apply(scale=True)
    
    # CRT Bezel (angled front)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0.04, 0.15))
    crt = bpy.context.active_object
    crt.scale = (0.14, 0.12, 0.07)
    crt.rotation_euler = (math.radians(-25), 0, 0)
    bpy.ops.object.transform_apply(rotation=True, scale=True)
    
    # Screen face
    bpy.ops.mesh.primitive_plane_add(size=1.0, location=(0, 0.035, 0.16))
    screen = bpy.context.active_object
    screen.scale = (0.11, 0.09, 1.0)
    screen.rotation_euler = (math.radians(-25), 0, 0)
    bpy.ops.object.transform_apply(rotation=True, scale=True)
    
    # Dual Copper Loops (Toruses on the back)
    for y_offset in [-0.07, 0.07]:
        bpy.ops.mesh.primitive_torus_add(
            major_radius=0.045, minor_radius=0.006,
            location=(0, y_offset, 0.22),
            rotation=(math.radians(90), 0, 0)
        )
        loop = bpy.context.active_object
        loop.scale = (1, 1, 1.3)
        bpy.ops.object.transform_apply(rotation=True, scale=True)
        
    # Knobs & Rotary Switch
    for x_pos in [-0.05, 0.0, 0.05]:
        bpy.ops.mesh.primitive_cylinder_add(
            radius=0.015, depth=0.02,
            location=(x_pos, -0.09, 0.13)
        )
        
    # Join all into one Static Mesh
    bpy.ops.object.select_all(action='SELECT')
    bpy.context.view_layer.objects.active = base
    bpy.ops.object.join()
    
    # UV Smart Project
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.smart_project(island_margin=0.02)
    bpy.ops.object.mode_set(mode='OBJECT')
    
    out_path = os.path.join(PROPS_DIR, "SM_Harmonic_Resonator.fbx")
    export_static_mesh(out_path, base)

# ==============================================================================
# 2. TRILIGHT FLASHLIGHT (Tactical 3-Emitter Bezel)
# ==============================================================================
def build_trilight_flashlight():
    print("=== Building Trilight Flashlight ===")
    reset_scene()
    
    # Main Body Cylinder (Handle)
    bpy.ops.mesh.primitive_cylinder_add(radius=0.032, depth=0.22, location=(0, 0, 0.11))
    handle = bpy.context.active_object
    handle.name = "SM_Trilight_Flashlight"
    bpy.ops.object.transform_apply(scale=True)
    
    # Knurled grip ring
    bpy.ops.mesh.primitive_cylinder_add(radius=0.035, depth=0.12, location=(0, 0, 0.09))
    
    # Head / Bezel flare
    bpy.ops.mesh.primitive_cone_add(radius1=0.032, radius2=0.058, depth=0.08, location=(0, 0, 0.26))
    
    # Outer Bezel Ring
    bpy.ops.mesh.primitive_cylinder_add(radius=0.058, depth=0.04, location=(0, 0, 0.32))
    
    # 3 Emitters (arranged radially at 120 degrees)
    for angle in [0, 120, 240]:
        rad = math.radians(angle)
        ex = math.cos(rad) * 0.025
        ey = math.sin(rad) * 0.025
        bpy.ops.mesh.primitive_cylinder_add(radius=0.016, depth=0.01, location=(ex, ey, 0.34))
        
    # Toggle switch on side
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.035, 0, 0.17))
    sw = bpy.context.active_object
    sw.scale = (0.01, 0.025, 0.04)
    bpy.ops.object.transform_apply(scale=True)
    
    # Tail cap
    bpy.ops.mesh.primitive_cylinder_add(radius=0.036, depth=0.03, location=(0, 0, 0.01))
    
    # Join into single mesh
    bpy.ops.object.select_all(action='SELECT')
    bpy.context.view_layer.objects.active = handle
    bpy.ops.object.join()
    
    # UV Unwrap
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.smart_project(island_margin=0.02)
    bpy.ops.object.mode_set(mode='OBJECT')
    
    out_path = os.path.join(PROPS_DIR, "SM_Trilight_Flashlight.fbx")
    export_static_mesh(out_path, handle)

# ==============================================================================
# 3. HYDROLITIS (Viscous Fluid Creature)
# ==============================================================================
def build_hydrolitis():
    print("=== Building Hydrolitis Entity ===")
    reset_scene()
    
    # Torso blob (contorted sludge)
    bpy.ops.mesh.primitive_uv_sphere_add(segments=32, ring_count=24, radius=0.6, location=(0, 0, 1.3))
    torso = bpy.context.active_object
    torso.name = "SK_Hydrolitis"
    torso.scale = (0.9, 0.7, 1.4)
    bpy.ops.object.transform_apply(scale=True)
    
    # Head puddle / skull silhouette
    bpy.ops.mesh.primitive_uv_sphere_add(segments=24, ring_count=16, radius=0.4, location=(0, 0.15, 2.1))
    head = bpy.context.active_object
    head.scale = (0.8, 1.0, 1.1)
    bpy.ops.object.transform_apply(scale=True)
    
    # Eye hollows
    for x_eye in [-0.15, 0.15]:
        bpy.ops.mesh.primitive_uv_sphere_add(segments=12, ring_count=8, radius=0.08, location=(x_eye, 0.45, 2.15))
        
    # Dripping tendril arms
    for x_arm, angle in [(-0.65, 20), (0.65, -20)]:
        bpy.ops.mesh.primitive_cylinder_add(radius=0.18, depth=1.2, location=(x_arm, 0, 1.0), rotation=(0, math.radians(angle), 0))
        # Drooping dripping hands
        bpy.ops.mesh.primitive_cone_add(radius1=0.18, radius2=0.04, depth=0.8, location=(x_arm * 1.3, 0, 0.3))
        
    # Viscous Base Puddle (spreading over floor)
    bpy.ops.mesh.primitive_cylinder_add(radius=1.3, depth=0.12, location=(0, 0, 0.06))
    puddle = bpy.context.active_object
    puddle.scale = (1.4, 1.2, 0.5)
    bpy.ops.object.transform_apply(scale=True)
    
    # Join into creature mesh
    bpy.ops.object.select_all(action='SELECT')
    bpy.context.view_layer.objects.active = torso
    bpy.ops.object.join()
    
    # Add Subdivision Surface & Displace for organic dripping sludge look
    sub = torso.modifiers.new("Subsurf", 'SUBSURF')
    sub.levels = 1
    bpy.ops.object.modifier_apply(modifier="Subsurf")
    
    # UV Unwrap
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.smart_project(island_margin=0.02)
    bpy.ops.object.mode_set(mode='OBJECT')
    
    out_path = os.path.join(BESTIARY_DIR, "SK_Hydrolitis.fbx")
    export_static_mesh(out_path, torso)

if __name__ == "__main__":
    build_harmonic_resonator()
    build_trilight_flashlight()
    build_hydrolitis()
    print("\n[SUCCESS] All 3D assets generated and exported cleanly!")
