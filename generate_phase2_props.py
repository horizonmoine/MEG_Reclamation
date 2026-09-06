import bpy
import bmesh
import math
import os
import sys

print('=== STARTING MEG PHASE 2 ASSET GENERATION ===')

OUTPUT_DIR = r'F:\MEG_Reclamation\RawAssets\FBX'
PROPS_DIR = os.path.join(OUTPUT_DIR, 'Props')
TOOLS_DIR = os.path.join(OUTPUT_DIR, 'Tools')
MODULAR_DIR = os.path.join(OUTPUT_DIR, 'Modular')

for d in [PROPS_DIR, TOOLS_DIR, MODULAR_DIR]:
    os.makedirs(d, exist_ok=True)

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
                bsdf.inputs['Emission Strength'].default_value = 2.0
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

def export_fbx(obj, filepath):
    for o in bpy.data.objects:
        o.select_set(False)
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    
    bpy.ops.export_scene.fbx(
        filepath=filepath,
        use_selection=True,
        global_scale=1.0,
        apply_unit_scale=True,
        bake_space_transform=True,
        object_types={'MESH'}
    )
    print(f'-> Exported: {filepath}')

# ==========================================
# 1. SM_Exit_Sign (Props)
# ==========================================
def make_exit_sign():
    reset_scene()
    m_housing = create_mat('Mat_ExitSign_Housing', (0.12, 0.12, 0.14, 1.0), roughness=0.35, metallic=0.7)
    m_diffuser = create_mat('Mat_ExitSign_Diffuser', (0.15, 0.95, 0.35, 1.0), roughness=0.1, metallic=0.0, emissive=(0.2, 1.0, 0.4, 1.0))
    m_led = create_mat('Mat_ExitSign_LED', (0.9, 0.1, 0.1, 1.0), roughness=0.2, emissive=(1.0, 0.1, 0.1, 1.0))

    # Main metal box housing (42cm x 8cm x 24cm)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.12))
    housing = bpy.context.active_object
    housing.scale = (0.42, 0.08, 0.24)
    bpy.ops.object.transform_apply(scale=True)

    # Front diffuser acrylic face (slightly protruding forward)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, -0.042, 0.12))
    diffuser = bpy.context.active_object
    diffuser.scale = (0.38, 0.01, 0.20)
    bpy.ops.object.transform_apply(scale=True)

    # Top ceiling mounting bracket & rods
    bpy.ops.mesh.primitive_cylinder_add(radius=0.012, depth=0.20, location=(-0.14, 0, 0.34))
    rod1 = bpy.context.active_object
    bpy.ops.mesh.primitive_cylinder_add(radius=0.012, depth=0.20, location=(0.14, 0, 0.34))
    rod2 = bpy.context.active_object

    # Top mounting plates
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(-0.14, 0, 0.44))
    plate1 = bpy.context.active_object
    plate1.scale = (0.06, 0.06, 0.01)
    bpy.ops.object.transform_apply(scale=True)

    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.14, 0, 0.44))
    plate2 = bpy.context.active_object
    plate2.scale = (0.06, 0.06, 0.01)
    bpy.ops.object.transform_apply(scale=True)

    # Bottom emergency indicator dot
    bpy.ops.mesh.primitive_cylinder_add(radius=0.008, depth=0.015, location=(0.16, -0.041, 0.02))
    dot = bpy.context.active_object
    dot.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)

    # Join
    all_parts = [housing, diffuser, rod1, rod2, plate1, plate2, dot]
    for p in all_parts:
        p.select_set(True)
    bpy.context.view_layer.objects.active = housing
    bpy.ops.object.join()

    finalize_mesh(housing, [m_housing, m_diffuser, m_led])
    export_fbx(housing, os.path.join(PROPS_DIR, 'SM_Exit_Sign.fbx'))

# ==========================================
# 2. SM_Partygoer_Balloon (Props)
# ==========================================
def make_partygoer_balloon():
    reset_scene()
    m_balloon = create_mat('Mat_Balloon_LatexRed', (0.92, 0.04, 0.04, 1.0), roughness=0.15, metallic=0.05)
    m_string = create_mat('Mat_Balloon_String', (0.85, 0.85, 0.80, 1.0), roughness=0.7)

    # Balloon teardrop body
    bpy.ops.mesh.primitive_uv_sphere_add(segments=24, ring_count=18, radius=0.16, location=(0, 0, 0.22))
    balloon = bpy.context.active_object
    balloon.scale = (1.0, 1.0, 1.25)
    bpy.ops.object.transform_apply(scale=True)

    # Tie knot at bottom
    bpy.ops.mesh.primitive_torus_add(major_radius=0.022, minor_radius=0.012, location=(0, 0, 0.015))
    knot = bpy.context.active_object

    # Hanging curled string (chain of cylinder segments curving down)
    string_parts = []
    points = [
        (0.00, 0.00, 0.00),
        (0.01, 0.01, -0.12),
        (-0.015, 0.02, -0.24),
        (0.02, -0.01, -0.38),
        (-0.01, -0.02, -0.52),
        (0.005, 0.01, -0.65)
    ]
    for i in range(len(points) - 1):
        p1 = points[i]
        p2 = points[i+1]
        mid = ((p1[0]+p2[0])*0.5, (p1[1]+p2[1])*0.5, (p1[2]+p2[2])*0.5)
        dx = p2[0] - p1[0]
        dy = p2[1] - p1[1]
        dz = p2[2] - p1[2]
        dist = math.sqrt(dx*dx + dy*dy + dz*dz)
        bpy.ops.mesh.primitive_cylinder_add(radius=0.003, depth=dist, location=mid)
        seg = bpy.context.active_object
        seg.rotation_euler = (math.atan2(math.sqrt(dx*dx+dy*dy), dz), 0, math.atan2(dy, dx))
        bpy.ops.object.transform_apply(rotation=True)
        string_parts.append(seg)

    for p in [balloon, knot] + string_parts:
        p.select_set(True)
    bpy.context.view_layer.objects.active = balloon
    bpy.ops.object.join()

    finalize_mesh(balloon, [m_balloon, m_string])
    export_fbx(balloon, os.path.join(PROPS_DIR, 'SM_Partygoer_Balloon.fbx'))

# ==========================================
# 3. SM_SupplyCrate_MEG (Props)
# ==========================================
def make_supply_crate_meg():
    reset_scene()
    m_body = create_mat('Mat_MEGCrate_OlivePolymer', (0.28, 0.32, 0.22, 1.0), roughness=0.45, metallic=0.05)
    m_trim = create_mat('Mat_Crate_BlackTrim', (0.08, 0.08, 0.09, 1.0), roughness=0.6, metallic=0.1)
    m_latches = create_mat('Mat_Crate_MetalLatches', (0.75, 0.75, 0.78, 1.0), roughness=0.25, metallic=0.9)

    # Main crate body (0.80m x 0.50m x 0.38m)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.19))
    body = bpy.context.active_object
    body.scale = (0.80, 0.50, 0.38)
    bpy.ops.object.transform_apply(scale=True)

    # Lid top rim
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.39))
    lid = bpy.context.active_object
    lid.scale = (0.82, 0.52, 0.05)
    bpy.ops.object.transform_apply(scale=True)

    parts = [body, lid]

    # Stiffening ribs on lid (4 longitudinal ribs)
    for i in [-0.28, -0.09, 0.09, 0.28]:
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(i, 0, 0.425))
        rib = bpy.context.active_object
        rib.scale = (0.05, 0.46, 0.025)
        bpy.ops.object.transform_apply(scale=True)
        parts.append(rib)

    # 4 Corner bumpers
    for x in [-0.40, 0.40]:
        for y in [-0.25, 0.25]:
            bpy.ops.mesh.primitive_cube_add(size=1.0, location=(x, y, 0.19))
            bumper = bpy.context.active_object
            bumper.scale = (0.06, 0.06, 0.39)
            bpy.ops.object.transform_apply(scale=True)
            parts.append(bumper)

    # 4 Toggle clasps (front left, front right, sides)
    for x in [-0.25, 0.25]:
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(x, -0.26, 0.36))
        latch = bpy.context.active_object
        latch.scale = (0.06, 0.02, 0.08)
        bpy.ops.object.transform_apply(scale=True)
        parts.append(latch)

    # Side handles
    for x, rot in [(-0.41, 0), (0.41, math.radians(180))]:
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(x, 0, 0.25))
        handle = bpy.context.active_object
        handle.scale = (0.04, 0.18, 0.04)
        bpy.ops.object.transform_apply(scale=True)
        parts.append(handle)

    for p in parts:
        p.select_set(True)
    bpy.context.view_layer.objects.active = body
    bpy.ops.object.join()

    finalize_mesh(body, [m_body, m_trim, m_latches])
    export_fbx(body, os.path.join(PROPS_DIR, 'SM_SupplyCrate_MEG.fbx'))

# ==========================================
# 4. SM_Medkit_MEG (Props)
# ==========================================
def make_medkit_meg():
    reset_scene()
    m_shell = create_mat('Mat_Medkit_Shell', (0.88, 0.86, 0.80, 1.0), roughness=0.35)
    m_cross = create_mat('Mat_Medkit_Cross', (0.85, 0.08, 0.08, 1.0), roughness=0.25)
    m_hard = create_mat('Mat_Medkit_Hardware', (0.15, 0.15, 0.18, 1.0), roughness=0.3, metallic=0.7)

    # Main briefcase case (0.36m x 0.28m x 0.14m)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.07))
    case = bpy.context.active_object
    case.scale = (0.36, 0.28, 0.14)
    bpy.ops.object.transform_apply(scale=True)

    # Raised medical cross on top lid
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.143))
    c_vert = bpy.context.active_object
    c_vert.scale = (0.04, 0.12, 0.008)
    bpy.ops.object.transform_apply(scale=True)

    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.143))
    c_horiz = bpy.context.active_object
    c_horiz.scale = (0.12, 0.04, 0.008)
    bpy.ops.object.transform_apply(scale=True)

    # Molded carry handle on front/top
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, -0.16, 0.07))
    handle = bpy.context.active_object
    handle.scale = (0.14, 0.03, 0.04)
    bpy.ops.object.transform_apply(scale=True)

    # 2 Front clasps
    latches = []
    for x in [-0.10, 0.10]:
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(x, -0.145, 0.07))
        latch = bpy.context.active_object
        latch.scale = (0.03, 0.015, 0.05)
        bpy.ops.object.transform_apply(scale=True)
        latches.append(latch)

    for p in [case, c_vert, c_horiz, handle] + latches:
        p.select_set(True)
    bpy.context.view_layer.objects.active = case
    bpy.ops.object.join()

    finalize_mesh(case, [m_shell, m_cross, m_hard])
    export_fbx(case, os.path.join(PROPS_DIR, 'SM_Medkit_MEG.fbx'))

# ==========================================
# 5. SM_AudioDecoy (Tools)
# ==========================================
def make_audio_decoy():
    reset_scene()
    m_body = create_mat('Mat_Decoy_Body', (0.16, 0.16, 0.18, 1.0), roughness=0.55, metallic=0.2)
    m_speaker = create_mat('Mat_Decoy_Speaker', (0.06, 0.06, 0.07, 1.0), roughness=0.35, metallic=0.8)
    m_details = create_mat('Mat_Decoy_Details', (0.85, 0.65, 0.20, 1.0), roughness=0.2, metallic=0.9, emissive=(0.8, 0.3, 0.05, 1.0))

    # Main rectangular chassis (0.18m x 0.08m x 0.22m)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.11))
    chassis = bpy.context.active_object
    chassis.scale = (0.18, 0.08, 0.22)
    bpy.ops.object.transform_apply(scale=True)

    # Front speaker circular grille
    bpy.ops.mesh.primitive_cylinder_add(radius=0.055, depth=0.015, location=(0, -0.042, 0.135))
    speaker = bpy.context.active_object
    speaker.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)

    # Cassette deck window below speaker
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, -0.041, 0.05))
    window = bpy.context.active_object
    window.scale = (0.12, 0.008, 0.06)
    bpy.ops.object.transform_apply(scale=True)

    # 2 Tape spools
    for x in [-0.035, 0.035]:
        bpy.ops.mesh.primitive_cylinder_add(radius=0.016, depth=0.01, location=(x, -0.042, 0.05))
        spool = bpy.context.active_object
        spool.rotation_euler = (math.radians(90), 0, 0)
        bpy.ops.object.transform_apply(rotation=True)

    # Top panel knobs & antenna
    bpy.ops.mesh.primitive_cylinder_add(radius=0.015, depth=0.025, location=(-0.05, 0, 0.23))
    knob1 = bpy.context.active_object

    bpy.ops.mesh.primitive_cylinder_add(radius=0.012, depth=0.02, location=(0.0, 0, 0.23))
    knob2 = bpy.context.active_object

    # Whip antenna
    bpy.ops.mesh.primitive_cylinder_add(radius=0.005, depth=0.28, location=(0.06, 0, 0.36))
    antenna = bpy.context.active_object
    antenna.rotation_euler = (math.radians(5), 0, math.radians(5))
    bpy.ops.object.transform_apply(rotation=True)

    for o in bpy.data.objects:
        o.select_set(True)
    bpy.context.view_layer.objects.active = chassis
    bpy.ops.object.join()

    finalize_mesh(chassis, [m_body, m_speaker, m_details])
    export_fbx(chassis, os.path.join(TOOLS_DIR, 'SM_AudioDecoy.fbx'))

# ==========================================
# 6. SM_AlmondWaterSpray (Tools)
# ==========================================
def make_almond_water_spray():
    reset_scene()
    m_tank = create_mat('Mat_Spray_Tank', (0.85, 0.72, 0.22, 1.0), roughness=0.35, metallic=0.1)
    m_brass = create_mat('Mat_Spray_Brass', (0.82, 0.68, 0.28, 1.0), roughness=0.25, metallic=0.85)
    m_gauge = create_mat('Mat_Spray_Gauge', (0.9, 0.9, 0.9, 1.0), roughness=0.1, metallic=0.0)

    # Main cylindrical tank (radius 0.10m, depth 0.34m)
    bpy.ops.mesh.primitive_cylinder_add(radius=0.10, depth=0.34, location=(0, 0, 0.17))
    tank = bpy.context.active_object

    # Top dome
    bpy.ops.mesh.primitive_uv_sphere_add(segments=16, ring_count=12, radius=0.10, location=(0, 0, 0.34))
    dome = bpy.context.active_object
    dome.scale = (1.0, 1.0, 0.4)
    bpy.ops.object.transform_apply(scale=True)

    # Plunger rod & T-handle
    bpy.ops.mesh.primitive_cylinder_add(radius=0.012, depth=0.14, location=(0, 0, 0.44))
    rod = bpy.context.active_object

    bpy.ops.mesh.primitive_cylinder_add(radius=0.014, depth=0.16, location=(0, 0, 0.51))
    handle = bpy.context.active_object
    handle.rotation_euler = (0, math.radians(90), 0)
    bpy.ops.object.transform_apply(rotation=True)

    # Pressure gauge dial on side
    bpy.ops.mesh.primitive_cylinder_add(radius=0.025, depth=0.02, location=(0.11, 0, 0.32))
    gauge = bpy.context.active_object
    gauge.rotation_euler = (0, math.radians(90), 0)
    bpy.ops.object.transform_apply(rotation=True)

    # Spray wand on side
    bpy.ops.mesh.primitive_cylinder_add(radius=0.008, depth=0.45, location=(-0.11, 0, 0.24))
    wand = bpy.context.active_object

    bpy.ops.mesh.primitive_cone_add(radius1=0.015, radius2=0.004, depth=0.04, location=(-0.11, 0, 0.48))
    nozzle = bpy.context.active_object

    for o in [tank, dome, rod, handle, gauge, wand, nozzle]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = tank
    bpy.ops.object.join()

    finalize_mesh(tank, [m_tank, m_brass, m_gauge])
    export_fbx(tank, os.path.join(TOOLS_DIR, 'SM_AlmondWaterSpray.fbx'))

# ==========================================
# 7. SM_Debris_Trash (Props)
# ==========================================
def make_debris_trash():
    reset_scene()
    m_paper = create_mat('Mat_Debris_Paper', (0.85, 0.82, 0.70, 1.0), roughness=0.85)
    m_plaster = create_mat('Mat_Debris_Plaster', (0.78, 0.76, 0.74, 1.0), roughness=0.92)
    m_metal = create_mat('Mat_Debris_Metal', (0.45, 0.42, 0.40, 1.0), roughness=0.4, metallic=0.6)

    parts = []

    # 3 Scattered curled paper sheets
    paper_coords = [
        (0.0, 0.0, 0.002, 0.28, 0.20, 15),
        (-0.18, 0.12, 0.004, 0.25, 0.18, -40),
        (0.16, -0.10, 0.003, 0.30, 0.21, 70)
    ]
    for x, y, z, sx, sy, yaw in paper_coords:
        bpy.ops.mesh.primitive_plane_add(size=1.0, location=(x, y, z))
        sheet = bpy.context.active_object
        sheet.scale = (sx, sy, 1.0)
        sheet.rotation_euler = (0, 0, math.radians(yaw))
        bpy.ops.object.transform_apply(scale=True, rotation=True)
        parts.append(sheet)

    # Broken plaster ceiling tile chunks
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(-0.10, -0.15, 0.03))
    chunk1 = bpy.context.active_object
    chunk1.scale = (0.16, 0.12, 0.05)
    chunk1.rotation_euler = (math.radians(8), math.radians(-12), math.radians(35))
    bpy.ops.object.transform_apply(scale=True, rotation=True)
    parts.append(chunk1)

    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.14, 0.18, 0.025))
    chunk2 = bpy.context.active_object
    chunk2.scale = (0.12, 0.10, 0.04)
    chunk2.rotation_euler = (math.radians(-5), math.radians(15), math.radians(-60))
    bpy.ops.object.transform_apply(scale=True, rotation=True)
    parts.append(chunk2)

    # Crushed soda can
    bpy.ops.mesh.primitive_cylinder_add(radius=0.03, depth=0.11, location=(0.04, 0.14, 0.03))
    can = bpy.context.active_object
    can.rotation_euler = (0, math.radians(90), math.radians(25))
    can.scale = (1.0, 0.6, 0.8) # dented
    bpy.ops.object.transform_apply(scale=True, rotation=True)
    parts.append(can)

    for p in parts:
        p.select_set(True)
    bpy.context.view_layer.objects.active = parts[0]
    bpy.ops.object.join()

    finalize_mesh(parts[0], [m_paper, m_plaster, m_metal])
    export_fbx(parts[0], os.path.join(PROPS_DIR, 'SM_Debris_Trash.fbx'))

# ==========================================
# 8. SM_Terminal_MEG (Props)
# ==========================================
def make_terminal_meg():
    reset_scene()
    m_case = create_mat('Mat_Terminal_SteelCase', (0.35, 0.36, 0.32, 1.0), roughness=0.5, metallic=0.3)
    m_screen = create_mat('Mat_Terminal_Screen', (0.02, 0.12, 0.08, 1.0), roughness=0.1, emissive=(0.05, 0.9, 0.4, 1.0))
    m_keys = create_mat('Mat_Terminal_Keys', (0.18, 0.18, 0.20, 1.0), roughness=0.6)

    # Heavy steel pedestal base (0.60m x 0.50m x 0.70m)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.35))
    pedestal = bpy.context.active_object
    pedestal.scale = (0.50, 0.40, 0.70)
    bpy.ops.object.transform_apply(scale=True)

    # Slanted console deck
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, -0.12, 0.75))
    deck = bpy.context.active_object
    deck.scale = (0.64, 0.32, 0.08)
    deck.rotation_euler = (math.radians(18), 0, 0)
    bpy.ops.object.transform_apply(scale=True, rotation=True)

    # Integrated keyboard block
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, -0.14, 0.77))
    kb = bpy.context.active_object
    kb.scale = (0.45, 0.18, 0.02)
    kb.rotation_euler = (math.radians(18), 0, 0)
    bpy.ops.object.transform_apply(scale=True, rotation=True)

    # CRT monitor enclosure (depth 0.42m, width 0.48m, height 0.38m)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0.08, 0.96))
    crt_box = bpy.context.active_object
    crt_box.scale = (0.48, 0.42, 0.38)
    bpy.ops.object.transform_apply(scale=True)

    # Curved CRT screen face
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, -0.135, 0.97))
    screen = bpy.context.active_object
    screen.scale = (0.38, 0.02, 0.28)
    bpy.ops.object.transform_apply(scale=True)

    for o in [pedestal, deck, kb, crt_box, screen]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = pedestal
    bpy.ops.object.join()

    finalize_mesh(pedestal, [m_case, m_screen, m_keys])
    export_fbx(pedestal, os.path.join(PROPS_DIR, 'SM_Terminal_MEG.fbx'))

# ==========================================
# EXECUTE PHASE 2 GENERATION
# ==========================================
try:
    print('Generating SM_Exit_Sign...')
    make_exit_sign()

    print('Generating SM_Partygoer_Balloon...')
    make_partygoer_balloon()

    print('Generating SM_SupplyCrate_MEG...')
    make_supply_crate_meg()

    print('Generating SM_Medkit_MEG...')
    make_medkit_meg()

    print('Generating SM_AudioDecoy...')
    make_audio_decoy()

    print('Generating SM_AlmondWaterSpray...')
    make_almond_water_spray()

    print('Generating SM_Debris_Trash...')
    make_debris_trash()

    print('Generating SM_Terminal_MEG...')
    make_terminal_meg()

    print('=== ALL 8 PHASE 2 ASSETS GENERATED SUCCESSFULLY ===')
except Exception as e:
    print(f'ERROR DURING PHASE 2 ASSET GENERATION: {e}')
    import traceback
    traceback.print_exc()
    sys.exit(1)
