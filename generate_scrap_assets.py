"""
================================================================================
M.E.G. RECLAMATION — Photorealistic 3D Scrap & Prop Generator (Blender DCC)
================================================================================
Generates optimized, detailed FBX 3D meshes with UVs and PBR materials for all
scavengeable loot items (style Lethal Company x Escape the Backrooms):
1. SM_Scrap_CopperCable (heavy wire spool)
2. SM_Battery_9V (industrial battery block)
3. SM_Retro_Computer (vintage 1980s portable CRT terminal)
4. SM_Declassified_Tape (classified MEG VHS cassette)
5. SM_Geiger_Counter (analog radiation survey meter)
6. SM_Anomalous_Core (glowing non-euclidean artifact)
7. SM_Heavy_Lead_Plate (35kg radiation shielding with handles)
8. SM_Scrap_Electronics (salvaged PCB motherboard)
"""

import bpy
import bmesh
import math
import os

OUTPUT_DIR = r"F:\MEG_Reclamation\RawAssets\FBX\Props"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    if bpy.context.scene.world is None:
        bpy.context.scene.world = bpy.data.worlds.new('World')

def create_mat(name, color, roughness=0.5, metallic=0.0, emissive=None):
    mat = bpy.data.materials.new(name=name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get('Principled BSDF')
    if bsdf:
        bsdf.inputs['Base Color'].default_value = color
        bsdf.inputs['Roughness'].default_value = roughness
        bsdf.inputs['Metallic'].default_value = metallic
        if emissive and 'Emission Color' in bsdf.inputs:
            bsdf.inputs['Emission Color'].default_value = emissive
            if 'Emission Strength' in bsdf.inputs:
                bsdf.inputs['Emission Strength'].default_value = 3.0
    return mat

def finalize_mesh(obj, mat_list=None):
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    
    if mat_list:
        for m in mat_list:
            obj.data.materials.append(m)
            
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.normals_make_consistent(inside=False)
    bpy.ops.uv.smart_project(angle_limit=math.radians(66.0), island_margin=0.02)
    bpy.ops.object.mode_set(mode='OBJECT')
    
    obj.data.polygons.foreach_set('use_smooth', [True] * len(obj.data.polygons))

def export_fbx(obj_list, filepath):
    for o in bpy.data.objects:
        o.select_set(False)
    for o in obj_list:
        o.select_set(True)
    bpy.context.view_layer.objects.active = obj_list[0]
    
    bpy.ops.export_scene.fbx(
        filepath=filepath,
        use_selection=True,
        global_scale=1.0,
        apply_unit_scale=True,
        bake_space_transform=True,
        object_types={'MESH'}
    )
    print(f"[3D] Exported: {filepath}")

# ==============================================================================
# 1. SM_Scrap_CopperCable (Heavy industrial cable drum / wire spool)
# ==============================================================================
def make_copper_cable():
    reset_scene()
    m_wood = create_mat("Mat_Spool_Wood", (0.38, 0.26, 0.16, 1.0), roughness=0.85, metallic=0.0)
    m_copper = create_mat("Mat_Scrap_Copper", (0.85, 0.45, 0.25, 1.0), roughness=0.30, metallic=0.95)
    
    # Wooden top & bottom flange disks (diameter 40cm, thickness 3cm, height 30cm)
    bpy.ops.mesh.primitive_cylinder_add(radius=0.20, depth=0.03, location=(0, 0, 0.015))
    flange_bottom = bpy.context.active_object
    
    bpy.ops.mesh.primitive_cylinder_add(radius=0.20, depth=0.03, location=(0, 0, 0.285))
    flange_top = bpy.context.active_object
    
    # Central wound copper coil cylinder (diameter 28cm, height 24cm)
    bpy.ops.mesh.primitive_cylinder_add(radius=0.14, depth=0.24, location=(0, 0, 0.15))
    copper_coil = bpy.context.active_object
    
    # Outer unwound copper wire loop
    bpy.ops.mesh.primitive_torus_add(major_radius=0.16, minor_radius=0.015, location=(0.02, 0, 0.10))
    wire_loose = bpy.context.active_object
    
    # Join
    for o in [flange_bottom, flange_top, copper_coil, wire_loose]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = flange_bottom
    bpy.ops.object.join()
    mesh = bpy.context.active_object
    mesh.name = "SM_Scrap_CopperCable"
    
    finalize_mesh(mesh, [m_wood, m_copper])
    export_fbx([mesh], os.path.join(OUTPUT_DIR, "SM_Scrap_CopperCable.fbx"))

# ==============================================================================
# 2. SM_Battery_9V (Industrial high-drain battery block)
# ==============================================================================
def make_battery_9v():
    reset_scene()
    m_casing = create_mat("Mat_Battery_Casing", (0.10, 0.10, 0.12, 1.0), roughness=0.45, metallic=0.1)
    m_gold = create_mat("Mat_Battery_LabelGold", (0.85, 0.70, 0.15, 1.0), roughness=0.35, metallic=0.8)
    m_metal = create_mat("Mat_Battery_Terminals", (0.75, 0.75, 0.78, 1.0), roughness=0.20, metallic=0.95)
    
    # Rectangular body (10cm x 6cm x 16cm)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.08))
    body = bpy.context.active_object
    body.scale = (0.10, 0.06, 0.16)
    bpy.ops.object.transform_apply(scale=True)
    
    # Terminals on top (+ circular terminal, - snap octagonal terminal)
    bpy.ops.mesh.primitive_cylinder_add(radius=0.012, depth=0.02, location=(-0.025, 0, 0.17))
    term_pos = bpy.context.active_object
    
    bpy.ops.mesh.primitive_cylinder_add(vertices=8, radius=0.015, depth=0.02, location=(0.025, 0, 0.17))
    term_neg = bpy.context.active_object
    
    # Join
    for o in [body, term_pos, term_neg]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = body
    bpy.ops.object.join()
    mesh = bpy.context.active_object
    mesh.name = "SM_Battery_9V"
    
    finalize_mesh(mesh, [m_casing, m_gold, m_metal])
    export_fbx([mesh], os.path.join(OUTPUT_DIR, "SM_Battery_9V.fbx"))

# ==============================================================================
# 3. SM_Retro_Computer (1980s portable CRT terminal unit - two-handed heavy scrap)
# ==============================================================================
def make_retro_computer():
    reset_scene()
    m_plastic = create_mat("Mat_RetroComp_Beige", (0.78, 0.74, 0.65, 1.0), roughness=0.60, metallic=0.0)
    m_crt = create_mat("Mat_RetroComp_CRT", (0.05, 0.08, 0.06, 1.0), roughness=0.08, metallic=0.1, emissive=(0.1, 0.4, 0.2, 1.0))
    m_keys = create_mat("Mat_RetroComp_Keys", (0.25, 0.24, 0.22, 1.0), roughness=0.50, metallic=0.0)
    m_handle = create_mat("Mat_RetroComp_Handle", (0.15, 0.15, 0.16, 1.0), roughness=0.30, metallic=0.85)
    
    # Main casing box (46cm x 36cm x 26cm)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.13))
    chassis = bpy.context.active_object
    chassis.scale = (0.46, 0.36, 0.26)
    bpy.ops.object.transform_apply(scale=True)
    
    # CRT screen bezel recessed into front (-Y)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(-0.08, -0.175, 0.16))
    crt = bpy.context.active_object
    crt.scale = (0.22, 0.02, 0.16)
    bpy.ops.object.transform_apply(scale=True)
    
    # Floppy drive slots on right side of screen
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.12, -0.175, 0.18))
    drive1 = bpy.context.active_object
    drive1.scale = (0.14, 0.02, 0.025)
    bpy.ops.object.transform_apply(scale=True)
    
    # Keyboard deck in front lower shelf
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, -0.175, 0.04))
    keyboard = bpy.context.active_object
    keyboard.scale = (0.40, 0.04, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    
    # Heavy carry handle on top (+Z)
    bpy.ops.mesh.primitive_cylinder_add(radius=0.015, depth=0.28, location=(0, 0, 0.28))
    handle = bpy.context.active_object
    handle.rotation_euler = (0, math.radians(90), 0)
    bpy.ops.object.transform_apply(rotation=True)
    
    for o in [chassis, crt, drive1, keyboard, handle]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = chassis
    bpy.ops.object.join()
    mesh = bpy.context.active_object
    mesh.name = "SM_Retro_Computer"
    
    finalize_mesh(mesh, [m_plastic, m_crt, m_keys, m_handle])
    export_fbx([mesh], os.path.join(OUTPUT_DIR, "SM_Retro_Computer.fbx"))

# ==============================================================================
# 4. SM_Declassified_Tape (Classified M.E.G. VHS cassette)
# ==============================================================================
def make_declassified_tape():
    reset_scene()
    m_plastic = create_mat("Mat_Tape_Black", (0.08, 0.08, 0.08, 1.0), roughness=0.40, metallic=0.05)
    m_label = create_mat("Mat_Tape_Label", (0.92, 0.90, 0.85, 1.0), roughness=0.80, metallic=0.0)
    m_reels = create_mat("Mat_Tape_Reels", (0.80, 0.80, 0.80, 1.0), roughness=0.15, metallic=0.0)
    
    # Standard VHS cassette: 18.7cm x 10.3cm x 2.5cm
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.0125))
    shell = bpy.context.active_object
    shell.scale = (0.187, 0.103, 0.025)
    bpy.ops.object.transform_apply(scale=True)
    
    # Paper label rectangle
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, -0.015, 0.0255))
    label = bpy.context.active_object
    label.scale = (0.14, 0.05, 0.002)
    bpy.ops.object.transform_apply(scale=True)
    
    # Left & right reel windows
    bpy.ops.mesh.primitive_cylinder_add(radius=0.025, depth=0.026, location=(-0.045, 0.015, 0.0125))
    reel_left = bpy.context.active_object
    
    bpy.ops.mesh.primitive_cylinder_add(radius=0.025, depth=0.026, location=(0.045, 0.015, 0.0125))
    reel_right = bpy.context.active_object
    
    for o in [shell, label, reel_left, reel_right]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = shell
    bpy.ops.object.join()
    mesh = bpy.context.active_object
    mesh.name = "SM_Declassified_Tape"
    
    finalize_mesh(mesh, [m_plastic, m_label, m_reels])
    export_fbx([mesh], os.path.join(OUTPUT_DIR, "SM_Declassified_Tape.fbx"))

# ==============================================================================
# 5. SM_Geiger_Counter (Analog radiation meter with probe)
# ==============================================================================
def make_geiger_counter():
    reset_scene()
    m_body = create_mat("Mat_Geiger_Yellow", (0.85, 0.72, 0.12, 1.0), roughness=0.40, metallic=0.7)
    m_dial = create_mat("Mat_Geiger_Dial", (0.95, 0.95, 0.90, 1.0), roughness=0.15, metallic=0.0)
    m_chrome = create_mat("Mat_Geiger_Chrome", (0.75, 0.75, 0.78, 1.0), roughness=0.18, metallic=0.95)
    
    # Box body (18cm x 12cm x 10cm)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.05))
    body = bpy.context.active_object
    body.scale = (0.18, 0.12, 0.10)
    bpy.ops.object.transform_apply(scale=True)
    
    # Top analog needle gauge (tilted face)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(-0.03, 0, 0.105))
    dial = bpy.context.active_object
    dial.scale = (0.08, 0.08, 0.02)
    bpy.ops.object.transform_apply(scale=True)
    
    # Top handle
    bpy.ops.mesh.primitive_cylinder_add(radius=0.01, depth=0.12, location=(0, 0, 0.14))
    handle = bpy.context.active_object
    handle.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    
    # Probe wand cylinder (attached to side)
    bpy.ops.mesh.primitive_cylinder_add(radius=0.018, depth=0.16, location=(0.11, 0, 0.06))
    probe = bpy.context.active_object
    
    for o in [body, dial, handle, probe]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = body
    bpy.ops.object.join()
    mesh = bpy.context.active_object
    mesh.name = "SM_Geiger_Counter"
    
    finalize_mesh(mesh, [m_body, m_dial, m_chrome])
    export_fbx([mesh], os.path.join(OUTPUT_DIR, "SM_Geiger_Counter.fbx"))

# ==============================================================================
# 6. SM_Anomalous_Core (Glowing non-euclidean artifact / tesseract core)
# ==============================================================================
def make_anomalous_core():
    reset_scene()
    m_brass = create_mat("Mat_Core_Brass", (0.80, 0.60, 0.20, 1.0), roughness=0.25, metallic=0.90)
    m_glow = create_mat("Mat_Core_CyanGlow", (0.10, 0.95, 1.00, 1.0), roughness=0.05, metallic=0.0, emissive=(0.3, 2.5, 3.5, 1.0))
    
    # Central faceted icosahedron
    bpy.ops.mesh.primitive_ico_sphere_add(radius=0.08, subdivisions=2, location=(0, 0, 0.12))
    crystal = bpy.context.active_object
    
    # Outer gimbal ring 1
    bpy.ops.mesh.primitive_torus_add(major_radius=0.12, minor_radius=0.012, location=(0, 0, 0.12))
    ring1 = bpy.context.active_object
    
    # Outer gimbal ring 2 (rotated 45 deg)
    bpy.ops.mesh.primitive_torus_add(major_radius=0.14, minor_radius=0.012, location=(0, 0, 0.12))
    ring2 = bpy.context.active_object
    ring2.rotation_euler = (math.radians(45), math.radians(30), 0)
    bpy.ops.object.transform_apply(rotation=True)
    
    for o in [crystal, ring1, ring2]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = crystal
    bpy.ops.object.join()
    mesh = bpy.context.active_object
    mesh.name = "SM_Anomalous_Core"
    
    finalize_mesh(mesh, [m_brass, m_glow])
    export_fbx([mesh], os.path.join(OUTPUT_DIR, "SM_Anomalous_Core.fbx"))

# ==============================================================================
# 7. SM_Heavy_Lead_Plate (35kg radiation shielding slab with carry handles)
# ==============================================================================
def make_lead_plate():
    reset_scene()
    m_lead = create_mat("Mat_Lead_Oxidized", (0.32, 0.33, 0.35, 1.0), roughness=0.65, metallic=0.85)
    m_handles = create_mat("Mat_Lead_SteelGrips", (0.12, 0.12, 0.14, 1.0), roughness=0.35, metallic=0.80)
    
    # Heavy slab (50cm x 40cm x 6cm)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.03))
    slab = bpy.context.active_object
    slab.scale = (0.50, 0.40, 0.06)
    bpy.ops.object.transform_apply(scale=True)
    
    # Left carry handle
    bpy.ops.mesh.primitive_torus_add(major_radius=0.04, minor_radius=0.012, location=(-0.18, 0, 0.07))
    handle_left = bpy.context.active_object
    handle_left.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    
    # Right carry handle
    bpy.ops.mesh.primitive_torus_add(major_radius=0.04, minor_radius=0.012, location=(0.18, 0, 0.07))
    handle_right = bpy.context.active_object
    handle_right.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    
    for o in [slab, handle_left, handle_right]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = slab
    bpy.ops.object.join()
    mesh = bpy.context.active_object
    mesh.name = "SM_Heavy_Lead_Plate"
    
    finalize_mesh(mesh, [m_lead, m_handles])
    export_fbx([mesh], os.path.join(OUTPUT_DIR, "SM_Heavy_Lead_Plate.fbx"))

# ==============================================================================
# 8. SM_Scrap_Electronics (Salvaged circuit motherboard with capacitors)
# ==============================================================================
def make_scrap_electronics():
    reset_scene()
    m_pcb = create_mat("Mat_PCB_Green", (0.08, 0.35, 0.12, 1.0), roughness=0.35, metallic=0.1)
    m_chips = create_mat("Mat_PCB_Chips", (0.10, 0.10, 0.10, 1.0), roughness=0.45, metallic=0.2)
    m_caps = create_mat("Mat_PCB_CapsBlue", (0.12, 0.35, 0.85, 1.0), roughness=0.25, metallic=0.7)
    
    # Circuit board plate (24cm x 18cm x 0.8cm)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.004))
    pcb = bpy.context.active_object
    pcb.scale = (0.24, 0.18, 0.008)
    bpy.ops.object.transform_apply(scale=True)
    
    # Main microprocessor chip in center
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.014))
    cpu = bpy.context.active_object
    cpu.scale = (0.06, 0.06, 0.012)
    bpy.ops.object.transform_apply(scale=True)
    
    # 4 cylindrical capacitors in row
    caps = []
    for i in range(4):
        bpy.ops.mesh.primitive_cylinder_add(radius=0.01, depth=0.03, location=(-0.08 + i * 0.025, 0.05, 0.02))
        caps.append(bpy.context.active_object)
        
    for o in [pcb, cpu] + caps:
        o.select_set(True)
    bpy.context.view_layer.objects.active = pcb
    bpy.ops.object.join()
    mesh = bpy.context.active_object
    mesh.name = "SM_Scrap_Electronics"
    
    finalize_mesh(mesh, [m_pcb, m_chips, m_caps])
    export_fbx([mesh], os.path.join(OUTPUT_DIR, "SM_Scrap_Electronics.fbx"))

print("=== STARTING SCRAP ASSET GENERATION IN BLENDER ===")
make_copper_cable()
make_battery_9v()
make_retro_computer()
make_declassified_tape()
make_geiger_counter()
make_anomalous_core()
make_lead_plate()
make_scrap_electronics()
print("=== ALL 8 SCRAP ASSETS SUCCESSFULLY GENERATED AND EXPORTED ===")
