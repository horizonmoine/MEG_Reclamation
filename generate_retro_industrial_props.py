import bpy
import bmesh
import math
import os
import sys

print("==========================================================")
print("=== MEG RECLAMATION: PROCEDURAL RETRO-INDUSTRIAL PROPS ===")
print("==========================================================")

OUTPUT_DIR = r"F:\MEG_Reclamation\RawAssets\FBX\Props"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    if bpy.context.scene.world is None:
        bpy.context.scene.world = bpy.data.worlds.new("World")

def create_mat(name, color, roughness=0.5, metallic=0.0, emissive=None):
    mat = bpy.data.materials.new(name=name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    if bsdf:
        bsdf.inputs["Base Color"].default_value = color
        bsdf.inputs["Roughness"].default_value = roughness
        bsdf.inputs["Metallic"].default_value = metallic
        if emissive:
            if "Emission Color" in bsdf.inputs:
                bsdf.inputs["Emission Color"].default_value = emissive
            if "Emission Strength" in bsdf.inputs:
                bsdf.inputs["Emission Strength"].default_value = 2.5
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

def export_fbx_with_collision(main_obj, collision_objs, filepath):
    for o in bpy.data.objects:
        o.select_set(False)
        
    main_obj.select_set(True)
    for c in collision_objs:
        c.select_set(True)
        
    bpy.context.view_layer.objects.active = main_obj
    
    bpy.ops.export_scene.fbx(
        filepath=filepath,
        use_selection=True,
        global_scale=1.0,
        apply_unit_scale=True,
        bake_space_transform=True,
        object_types={'MESH'}
    )
    print(f"-> Exported successfully: {filepath}")

# ==============================================================================
# 1. SM_Vending_Machine (Distributeur de snacks vintage decrepit)
# ==============================================================================
def make_vending_machine():
    reset_scene()
    print("Generating SM_Vending_Machine...")
    
    m_body = create_mat("Mat_Vending_FadedEnamel", (0.22, 0.32, 0.30, 1.0), roughness=0.55, metallic=0.25)
    m_metal = create_mat("Mat_Vending_DarkSteel", (0.15, 0.15, 0.16, 1.0), roughness=0.35, metallic=0.85)
    m_glass = create_mat("Mat_Vending_SmokedGlass", (0.75, 0.82, 0.85, 0.35), roughness=0.15, metallic=0.1)
    m_marquee = create_mat("Mat_Vending_MarqueeEmissive", (0.95, 0.85, 0.65, 1.0), roughness=0.2, emissive=(1.0, 0.88, 0.60, 1.0))
    m_coils = create_mat("Mat_Vending_CoilChrome", (0.85, 0.85, 0.88, 1.0), roughness=0.2, metallic=0.95)
    m_snack_red = create_mat("Mat_Vending_SnackRed", (0.8, 0.15, 0.12, 1.0), roughness=0.4)
    m_snack_yellow = create_mat("Mat_Vending_SnackYellow", (0.85, 0.72, 0.10, 1.0), roughness=0.4)
    m_keypad = create_mat("Mat_Vending_KeypadRubber", (0.1, 0.1, 0.12, 1.0), roughness=0.7)

    parts = []

    # 1. Main outer cabinet shell (0.95m wide x 0.85m deep x 1.95m high)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.975))
    cabinet = bpy.context.active_object
    cabinet.scale = (0.95, 0.85, 1.95)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(cabinet)

    # 2. Recessed base / kickplate (0.91m x 0.80m x 0.12m)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.06))
    kickplate = bpy.context.active_object
    kickplate.scale = (0.91, 0.80, 0.12)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(kickplate)

    # 4 Leveling feet / caster pads
    for fx in [-0.40, 0.40]:
        for fy in [-0.34, 0.34]:
            bpy.ops.mesh.primitive_cylinder_add(radius=0.045, depth=0.04, location=(fx, fy, 0.02))
            foot = bpy.context.active_object
            parts.append(foot)

    # 3. Top marquee lightbox header (Z=1.80m, front face at Y=-0.43m)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, -0.43, 1.80))
    marquee = bpy.context.active_object
    marquee.scale = (0.88, 0.05, 0.20)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(marquee)

    # Marquee sign border frame
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, -0.455, 1.80))
    m_frame = bpy.context.active_object
    m_frame.scale = (0.90, 0.02, 0.22)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(m_frame)

    # 4. Display Window Recess (left side: X from -0.42 to +0.14, Z from 0.65 to 1.68)
    # Window glass pane
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(-0.14, -0.435, 1.16))
    glass = bpy.context.active_object
    glass.scale = (0.58, 0.015, 1.04)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(glass)

    # Window heavy metal perimeter bezel
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(-0.14, -0.43, 1.16))
    w_bezel = bpy.context.active_object
    w_bezel.scale = (0.62, 0.03, 1.08)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(w_bezel)

    # 4 Snack shelves inside display cavity
    for sz in [0.75, 0.95, 1.15, 1.35]:
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(-0.14, -0.28, sz))
        shelf = bpy.context.active_object
        shelf.scale = (0.54, 0.26, 0.02)
        bpy.ops.object.transform_apply(scale=True)
        parts.append(shelf)

        # Dispense coils on each shelf (4 per shelf)
        for cx in [-0.34, -0.21, -0.07, 0.06]:
            bpy.ops.mesh.primitive_cylinder_add(radius=0.035, depth=0.22, location=(cx, -0.28, sz + 0.045))
            coil = bpy.context.active_object
            coil.rotation_euler = (math.radians(90), 0, 0)
            bpy.ops.object.transform_apply(rotation=True)
            parts.append(coil)

            # Snack items in coils (tilted chip bags / candy packs)
            bpy.ops.mesh.primitive_cube_add(size=1.0, location=(cx + 0.01, -0.27, sz + 0.065))
            snack = bpy.context.active_object
            snack.scale = (0.055, 0.03, 0.08)
            snack.rotation_euler = (math.radians(12), math.radians(5), math.radians(-8))
            bpy.ops.object.transform_apply(scale=True, rotation=True)
            parts.append(snack)

    # 5. Right Control Column (X = +0.28m, Y = -0.44m)
    # Recessed control panel backing plate
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.28, -0.435, 1.16))
    ctrl_plate = bpy.context.active_object
    ctrl_plate.scale = (0.24, 0.02, 1.04)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(ctrl_plate)

    # Digital price LED display window
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.28, -0.45, 1.50))
    led_disp = bpy.context.active_object
    led_disp.scale = (0.16, 0.015, 0.06)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(led_disp)

    # Coin slot casting bezel
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.28, -0.45, 1.36))
    coin_bezel = bpy.context.active_object
    coin_bezel.scale = (0.12, 0.02, 0.08)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(coin_bezel)

    # Coin slot inlet
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.28, -0.462, 1.36))
    coin_slot = bpy.context.active_object
    coin_slot.scale = (0.04, 0.01, 0.008)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(coin_slot)

    # Dollar bill validator intake bezel
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.28, -0.45, 1.22))
    bill_bezel = bpy.context.active_object
    bill_bezel.scale = (0.15, 0.025, 0.07)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(bill_bezel)

    # Keypad buttons (3x4 grid of selector keys)
    for row in range(4):
        for col in range(3):
            kx = 0.23 + col * 0.05
            kz = 1.08 - row * 0.055
            bpy.ops.mesh.primitive_cube_add(size=1.0, location=(kx, -0.452, kz))
            btn = bpy.context.active_object
            btn.scale = (0.035, 0.012, 0.035)
            bpy.ops.object.transform_apply(scale=True)
            parts.append(btn)

    # Change return cup cavity
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.28, -0.42, 0.72))
    cup = bpy.context.active_object
    cup.scale = (0.14, 0.07, 0.12)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(cup)

    # 6. Bottom Delivery Chute / Push Flap Bin (Z=0.22m to 0.52m, X=-0.14m)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(-0.14, -0.44, 0.38))
    bin_bezel = bpy.context.active_object
    bin_bezel.scale = (0.64, 0.04, 0.30)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(bin_bezel)

    # Swinging push flap door (angled slightly inward for decrepit feel)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(-0.14, -0.41, 0.38))
    bin_door = bpy.context.active_object
    bin_door.scale = (0.58, 0.015, 0.24)
    bin_door.rotation_euler = (math.radians(-14), 0, 0)
    bpy.ops.object.transform_apply(scale=True, rotation=True)
    parts.append(bin_door)

    # Horizontal handle bar on flap
    bpy.ops.mesh.primitive_cylinder_add(radius=0.012, depth=0.48, location=(-0.14, -0.43, 0.44))
    b_bar = bpy.context.active_object
    b_bar.rotation_euler = (0, math.radians(90), 0)
    bpy.ops.object.transform_apply(rotation=True)
    parts.append(b_bar)

    # 7. Compressor ventilation louvers at rear/bottom
    for i in range(5):
        lz = 0.22 + i * 0.05
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.25, 0.428, lz))
        grill = bpy.context.active_object
        grill.scale = (0.35, 0.015, 0.02)
        grill.rotation_euler = (math.radians(30), 0, 0)
        bpy.ops.object.transform_apply(scale=True, rotation=True)
        parts.append(grill)

    # Join visual mesh
    for p in parts:
        p.select_set(True)
    bpy.context.view_layer.objects.active = cabinet
    bpy.ops.object.join()
    cabinet.name = "SM_Vending_Machine"
    finalize_mesh(cabinet, [m_body, m_metal, m_glass, m_marquee, m_coils, m_snack_red, m_snack_yellow, m_keypad])

    # 8. Physical Collision: UCX convex box
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.975))
    ucx = bpy.context.active_object
    ucx.name = "UCX_SM_Vending_Machine_01"
    ucx.scale = (0.97, 0.87, 1.97)
    bpy.ops.object.transform_apply(scale=True)

    export_fbx_with_collision(cabinet, [ucx], os.path.join(OUTPUT_DIR, "SM_Vending_Machine.fbx"))

# ==============================================================================
# 2. SM_Vent_Duct_Straight (Conduit de ventilation droit)
# ==============================================================================
def make_vent_duct_straight():
    reset_scene()
    print("Generating SM_Vent_Duct_Straight...")
    
    m_galv = create_mat("Mat_Duct_GalvanizedSteel", (0.68, 0.70, 0.73, 1.0), roughness=0.38, metallic=0.82)
    m_hardware = create_mat("Mat_Duct_IronHardware", (0.22, 0.23, 0.25, 1.0), roughness=0.45, metallic=0.75)
    
    parts = []

    # Main rectangular duct (2.00m length along Y, 0.80m wide along X, 0.50m high along Z)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0))
    duct = bpy.context.active_object
    duct.scale = (0.80, 2.00, 0.50)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(duct)

    # Front and rear connection flanges (Y = -0.98m and Y = +0.98m)
    for fy in [-0.98, 0.98]:
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, fy, 0))
        flange = bpy.context.active_object
        flange.scale = (0.86, 0.04, 0.56)
        bpy.ops.object.transform_apply(scale=True)
        parts.append(flange)

        # Flange perimeter corner bolts (4 corner bolts per flange)
        for bx in [-0.40, 0.40]:
            for bz in [-0.25, 0.25]:
                bpy.ops.mesh.primitive_cylinder_add(radius=0.016, depth=0.06, location=(bx, fy, bz))
                bolt = bpy.context.active_object
                bolt.rotation_euler = (math.radians(90), 0, 0)
                bpy.ops.object.transform_apply(rotation=True)
                parts.append(bolt)

    # Intermediate sheet metal standing seams / reinforcement bands (at Y = -0.33m and Y = +0.33m)
    for ry in [-0.33, 0.33]:
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, ry, 0))
        rib = bpy.context.active_object
        rib.scale = (0.82, 0.03, 0.52)
        bpy.ops.object.transform_apply(scale=True)
        parts.append(rib)

    # Longitudinal stiffener ribs on top and bottom faces
    for sz in [-0.255, 0.255]:
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, sz))
        rib_long = bpy.context.active_object
        rib_long.scale = (0.04, 1.90, 0.015)
        bpy.ops.object.transform_apply(scale=True)
        parts.append(rib_long)

    # Trapeze ceiling suspension hanger assembly in the middle (Y = 0)
    # Unistrut bottom cross channel
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, -0.275))
    strut = bpy.context.active_object
    strut.scale = (0.96, 0.05, 0.04)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(strut)

    # Two threaded vertical drop rods rising up to ceiling anchor (Z = +0.55m)
    for rx in [-0.45, 0.45]:
        bpy.ops.mesh.primitive_cylinder_add(radius=0.012, depth=0.85, location=(rx, 0, 0.15))
        rod = bpy.context.active_object
        parts.append(rod)

        # Ceiling mounting foot bracket at top of rod
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(rx, 0, 0.575))
        anchor = bpy.context.active_object
        anchor.scale = (0.08, 0.08, 0.015)
        bpy.ops.object.transform_apply(scale=True)
        parts.append(anchor)

        # Retention nuts at bottom under strut
        bpy.ops.mesh.primitive_cylinder_add(radius=0.02, depth=0.025, location=(rx, 0, -0.305))
        nut = bpy.context.active_object
        parts.append(nut)

    # Join visual mesh
    for p in parts:
        p.select_set(True)
    bpy.context.view_layer.objects.active = duct
    bpy.ops.object.join()
    duct.name = "SM_Vent_Duct_Straight"
    finalize_mesh(duct, [m_galv, m_hardware])

    # Collision UCX box
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0))
    ucx = bpy.context.active_object
    ucx.name = "UCX_SM_Vent_Duct_Straight_01"
    ucx.scale = (0.88, 2.02, 0.58)
    bpy.ops.object.transform_apply(scale=True)

    export_fbx_with_collision(duct, [ucx], os.path.join(OUTPUT_DIR, "SM_Vent_Duct_Straight.fbx"))

# ==============================================================================
# 3. SM_Vent_Duct_Corner (Conduit de ventilation coude a 90 degres)
# ==============================================================================
def make_vent_duct_corner():
    reset_scene()
    print("Generating SM_Vent_Duct_Corner...")
    
    m_galv = create_mat("Mat_Duct_GalvanizedSteel", (0.68, 0.70, 0.73, 1.0), roughness=0.38, metallic=0.82)
    m_hardware = create_mat("Mat_Duct_IronHardware", (0.22, 0.23, 0.25, 1.0), roughness=0.45, metallic=0.75)

    parts = []

    # 90-degree mitered HVAC duct elbow:
    # Corner turns from entrance along Y (at Y = -0.90m, X = 0.0m) to exit along X (at X = 0.90m, Y = 0.0m).
    # Cross-section: 0.80m wide, 0.50m high.
    
    # Segment 1: Entering leg along Y
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.0, -0.50, 0))
    seg1 = bpy.context.active_object
    seg1.scale = (0.80, 0.80, 0.50)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(seg1)

    # Segment 2: 45-degree miter corner junction box
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.0, 0.0, 0))
    seg2 = bpy.context.active_object
    seg2.scale = (0.80, 0.80, 0.50)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(seg2)

    # Segment 3: Exiting leg along X
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.50, 0.0, 0))
    seg3 = bpy.context.active_object
    seg3.scale = (0.80, 0.80, 0.50)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(seg3)

    # End Flange 1 (at Y = -0.90m, X = 0.0m)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.0, -0.90, 0))
    flange1 = bpy.context.active_object
    flange1.scale = (0.86, 0.04, 0.56)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(flange1)

    for bx in [-0.40, 0.40]:
        for bz in [-0.25, 0.25]:
            bpy.ops.mesh.primitive_cylinder_add(radius=0.016, depth=0.06, location=(bx, -0.90, bz))
            b = bpy.context.active_object
            b.rotation_euler = (math.radians(90), 0, 0)
            bpy.ops.object.transform_apply(rotation=True)
            parts.append(b)

    # End Flange 2 (at X = 0.90m, Y = 0.0m)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.90, 0.0, 0))
    flange2 = bpy.context.active_object
    flange2.scale = (0.04, 0.86, 0.56)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(flange2)

    for by in [-0.40, 0.40]:
        for bz in [-0.25, 0.25]:
            bpy.ops.mesh.primitive_cylinder_add(radius=0.016, depth=0.06, location=(0.90, by, bz))
            b = bpy.context.active_object
            b.rotation_euler = (0, math.radians(90), 0)
            bpy.ops.object.transform_apply(rotation=True)
            parts.append(b)

    # Corner diagonal reinforcement gusset bracket (triangular/beveled corner stiffener)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(-0.38, -0.38, 0))
    gusset = bpy.context.active_object
    gusset.scale = (0.12, 0.12, 0.52)
    gusset.rotation_euler = (0, 0, math.radians(45))
    bpy.ops.object.transform_apply(scale=True, rotation=True)
    parts.append(gusset)

    # Corner ceiling hanger assembly:
    # 2 diagonal unistrut braces under corner
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.20, -0.20, -0.275))
    c_strut = bpy.context.active_object
    c_strut.scale = (0.90, 0.06, 0.04)
    c_strut.rotation_euler = (0, 0, math.radians(45))
    bpy.ops.object.transform_apply(scale=True, rotation=True)
    parts.append(c_strut)

    # 3 Threaded ceiling drop rods
    for pos in [(-0.35, 0.35), (0.35, -0.35), (0.45, 0.45)]:
        bpy.ops.mesh.primitive_cylinder_add(radius=0.012, depth=0.85, location=(pos[0], pos[1], 0.15))
        rod = bpy.context.active_object
        parts.append(rod)

        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(pos[0], pos[1], 0.575))
        anchor = bpy.context.active_object
        anchor.scale = (0.08, 0.08, 0.015)
        bpy.ops.object.transform_apply(scale=True)
        parts.append(anchor)

    # Join visual mesh
    for p in parts:
        p.select_set(True)
    bpy.context.view_layer.objects.active = seg1
    bpy.ops.object.join()
    seg1.name = "SM_Vent_Duct_Corner"
    finalize_mesh(seg1, [m_galv, m_hardware])

    # Two convex collision boxes for L-turn
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.0, -0.45, 0))
    ucx1 = bpy.context.active_object
    ucx1.name = "UCX_SM_Vent_Duct_Corner_01"
    ucx1.scale = (0.86, 0.95, 0.56)
    bpy.ops.object.transform_apply(scale=True)

    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.45, 0.0, 0))
    ucx2 = bpy.context.active_object
    ucx2.name = "UCX_SM_Vent_Duct_Corner_02"
    ucx2.scale = (0.95, 0.86, 0.56)
    bpy.ops.object.transform_apply(scale=True)

    export_fbx_with_collision(seg1, [ucx1, ucx2], os.path.join(OUTPUT_DIR, "SM_Vent_Duct_Corner.fbx"))

# ==============================================================================
# 4. SM_Emergency_Breaker (Panneau electrique de secours avec leviers)
# ==============================================================================
def make_emergency_breaker():
    reset_scene()
    print("Generating SM_Emergency_Breaker...")
    
    m_box = create_mat("Mat_Breaker_IndustrialOrange", (0.82, 0.42, 0.12, 1.0), roughness=0.5, metallic=0.3)
    m_metal = create_mat("Mat_Breaker_CastSteel", (0.18, 0.18, 0.20, 1.0), roughness=0.35, metallic=0.85)
    m_handle = create_mat("Mat_Breaker_EmergencyRed", (0.88, 0.08, 0.06, 1.0), roughness=0.4, metallic=0.1)
    m_dial = create_mat("Mat_Breaker_GaugeFace", (0.92, 0.90, 0.85, 1.0), roughness=0.3)
    m_red_led = create_mat("Mat_Breaker_RedPilot", (1.0, 0.1, 0.1, 1.0), roughness=0.15, emissive=(1.0, 0.05, 0.05, 1.0))
    m_green_led = create_mat("Mat_Breaker_GreenPilot", (0.1, 0.95, 0.2, 1.0), roughness=0.15, emissive=(0.1, 1.0, 0.2, 1.0))
    m_brass = create_mat("Mat_Breaker_BrassCoupling", (0.78, 0.65, 0.25, 1.0), roughness=0.3, metallic=0.9)

    parts = []

    # 1. Main cabinet box (0.65m wide x 0.26m deep x 0.90m high)
    # Origin back against wall at Y = 0
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0.13, 0.45))
    box = bpy.context.active_object
    box.scale = (0.60, 0.24, 0.85)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(box)

    # 4 Wall mounting tabs with lag bolts
    for mx in [-0.32, 0.32]:
        for mz in [0.08, 0.82]:
            bpy.ops.mesh.primitive_cube_add(size=1.0, location=(mx, 0.015, mz))
            tab = bpy.context.active_object
            tab.scale = (0.05, 0.03, 0.08)
            bpy.ops.object.transform_apply(scale=True)
            parts.append(tab)

            bpy.ops.mesh.primitive_cylinder_add(radius=0.015, depth=0.04, location=(mx, 0.03, mz))
            bolt = bpy.context.active_object
            bolt.rotation_euler = (math.radians(90), 0, 0)
            bpy.ops.object.transform_apply(rotation=True)
            parts.append(bolt)

    # 2. Hinged front door panel (front face at Y = 0.255m)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0.255, 0.45))
    door = bpy.context.active_object
    door.scale = (0.58, 0.025, 0.83)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(door)

    # Door perimeter gasket seal rim
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0.268, 0.45))
    rim = bpy.context.active_object
    rim.scale = (0.55, 0.01, 0.80)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(rim)

    # Heavy industrial barrel hinges on left side
    for hz in [0.22, 0.68]:
        bpy.ops.mesh.primitive_cylinder_add(radius=0.02, depth=0.10, location=(-0.30, 0.25, hz))
        hinge = bpy.context.active_object
        parts.append(hinge)

    # Heavy rotary latch & padlock hasp on right side
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.28, 0.275, 0.45))
    latch = bpy.context.active_object
    latch.scale = (0.05, 0.03, 0.12)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(latch)

    bpy.ops.mesh.primitive_cylinder_add(radius=0.015, depth=0.06, location=(0.28, 0.29, 0.45))
    knob = bpy.context.active_object
    knob.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    parts.append(knob)

    # 3. Warning chevron sign placard in center
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0.275, 0.30))
    hazard = bpy.context.active_object
    hazard.scale = (0.28, 0.008, 0.22)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(hazard)

    # 4. Two Analog Meters (Voltmeter & Ammeter) on upper door
    for gx in [-0.14, 0.14]:
        # Meter bezel
        bpy.ops.mesh.primitive_cylinder_add(radius=0.065, depth=0.025, location=(gx, 0.275, 0.65))
        m_bezel = bpy.context.active_object
        m_bezel.rotation_euler = (math.radians(90), 0, 0)
        bpy.ops.object.transform_apply(rotation=True)
        parts.append(m_bezel)

        # Dial face
        bpy.ops.mesh.primitive_cylinder_add(radius=0.052, depth=0.008, location=(gx, 0.285, 0.65))
        m_face = bpy.context.active_object
        m_face.rotation_euler = (math.radians(90), 0, 0)
        bpy.ops.object.transform_apply(rotation=True)
        parts.append(m_face)

        # Pointer needle
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(gx + 0.01, 0.29, 0.655))
        needle = bpy.context.active_object
        needle.scale = (0.035, 0.004, 0.004)
        needle.rotation_euler = (0, math.radians(35), 0)
        bpy.ops.object.transform_apply(scale=True, rotation=True)
        parts.append(needle)

    # 5. Top Pilot Indicator Lamps (Red & Green domes)
    # Red alarm lamp
    bpy.ops.mesh.primitive_cylinder_add(radius=0.025, depth=0.035, location=(-0.16, 0.13, 0.89))
    red_lamp = bpy.context.active_object
    parts.append(red_lamp)

    bpy.ops.mesh.primitive_uv_sphere_add(radius=0.022, location=(-0.16, 0.13, 0.91))
    red_dome = bpy.context.active_object
    parts.append(red_dome)

    # Green operational lamp
    bpy.ops.mesh.primitive_cylinder_add(radius=0.025, depth=0.035, location=(0.16, 0.13, 0.89))
    green_lamp = bpy.context.active_object
    parts.append(green_lamp)

    bpy.ops.mesh.primitive_uv_sphere_add(radius=0.022, location=(0.16, 0.13, 0.91))
    green_dome = bpy.context.active_object
    parts.append(green_dome)

    # 6. Prominent External Knife / Emergency Throw Lever on Right Flank
    # Pivot housing bracket on right side
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.32, 0.14, 0.48))
    p_box = bpy.context.active_object
    p_box.scale = (0.04, 0.08, 0.12)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(p_box)

    # Arched throw guide arc plate
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.33, 0.14, 0.54))
    guide = bpy.context.active_object
    guide.scale = (0.025, 0.16, 0.08)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(guide)

    # Steel lever pivot arm (angled at 40 degrees)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.35, 0.16, 0.52))
    l_arm = bpy.context.active_object
    l_arm.scale = (0.025, 0.04, 0.28)
    l_arm.rotation_euler = (math.radians(-38), 0, 0)
    bpy.ops.object.transform_apply(scale=True, rotation=True)
    parts.append(l_arm)

    # Red rubberized handle grip cylinder
    bpy.ops.mesh.primitive_cylinder_add(radius=0.026, depth=0.14, location=(0.35, 0.26, 0.64))
    grip = bpy.context.active_object
    grip.rotation_euler = (math.radians(52), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    parts.append(grip)

    # 7. Conduit gland fittings (2 top, 1 bottom)
    for cx in [-0.18, 0.18]:
        bpy.ops.mesh.primitive_cylinder_add(radius=0.035, depth=0.08, location=(cx, 0.13, 0.90))
        gland = bpy.context.active_object
        parts.append(gland)

        bpy.ops.mesh.primitive_cylinder_add(radius=0.025, depth=0.15, location=(cx, 0.13, 0.98))
        pipe = bpy.context.active_object
        parts.append(pipe)

    # Bottom main armored cable conduit
    bpy.ops.mesh.primitive_cylinder_add(radius=0.045, depth=0.08, location=(0.0, 0.13, 0.0))
    b_gland = bpy.context.active_object
    parts.append(b_gland)

    bpy.ops.mesh.primitive_cylinder_add(radius=0.035, depth=0.16, location=(0.0, 0.13, -0.08))
    b_pipe = bpy.context.active_object
    parts.append(b_pipe)

    # Join visual mesh
    for p in parts:
        p.select_set(True)
    bpy.context.view_layer.objects.active = box
    bpy.ops.object.join()
    box.name = "SM_Emergency_Breaker"
    finalize_mesh(box, [m_box, m_metal, m_handle, m_dial, m_red_led, m_green_led, m_brass])

    # Collision UCX box
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.03, 0.15, 0.45))
    ucx = bpy.context.active_object
    ucx.name = "UCX_SM_Emergency_Breaker_01"
    ucx.scale = (0.72, 0.32, 0.95)
    bpy.ops.object.transform_apply(scale=True)

    export_fbx_with_collision(box, [ucx], os.path.join(OUTPUT_DIR, "SM_Emergency_Breaker.fbx"))

# ==============================================================================
# 5. SM_Loot_Cart (Chariot de transport a roulettes pour le loot lourd)
# ==============================================================================
def make_loot_cart():
    reset_scene()
    print("Generating SM_Loot_Cart...")
    
    m_frame = create_mat("Mat_Cart_SafetyYellow", (0.88, 0.65, 0.12, 1.0), roughness=0.45, metallic=0.35)
    m_deck = create_mat("Mat_Cart_TreadSteel", (0.35, 0.35, 0.38, 1.0), roughness=0.5, metallic=0.85)
    m_rubber = create_mat("Mat_Cart_VulcanizedRubber", (0.1, 0.1, 0.1, 1.0), roughness=0.85, metallic=0.05)
    m_hardware = create_mat("Mat_Cart_GalvHardware", (0.75, 0.75, 0.78, 1.0), roughness=0.25, metallic=0.9)

    parts = []

    # 1. Main perimeter chassis frame (1.25m long X, 0.70m wide Y, at Z = 0.20m)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.20))
    chassis = bpy.context.active_object
    chassis.scale = (1.25, 0.70, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(chassis)

    # Diamond tread bed deck plate (1.21m x 0.66m x 0.015m)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.23))
    deck = bpy.context.active_object
    deck.scale = (1.21, 0.66, 0.015)
    bpy.ops.object.transform_apply(scale=True)
    parts.append(deck)

    # 4 Corner rubber bumper guards
    for cx in [-0.62, 0.62]:
        for cy in [-0.34, 0.34]:
            bpy.ops.mesh.primitive_cylinder_add(radius=0.045, depth=0.07, location=(cx, cy, 0.20))
            bumper = bpy.context.active_object
            parts.append(bumper)

    # 2. Tubular Push Handle Assembly at rear (X = -0.60m)
    # Upright tubes
    for hy in [-0.28, 0.28]:
        bpy.ops.mesh.primitive_cylinder_add(radius=0.022, depth=0.75, location=(-0.60, hy, 0.60))
        post = bpy.context.active_object
        parts.append(post)

        # Diagonal support braces
        bpy.ops.mesh.primitive_cylinder_add(radius=0.016, depth=0.45, location=(-0.45, hy, 0.35))
        brace = bpy.context.active_object
        brace.rotation_euler = (0, math.radians(-42), 0)
        bpy.ops.object.transform_apply(rotation=True)
        parts.append(brace)

    # Top horizontal ergonomic push bar
    bpy.ops.mesh.primitive_cylinder_add(radius=0.024, depth=0.64, location=(-0.60, 0, 0.96))
    hbar = bpy.context.active_object
    hbar.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    parts.append(hbar)

    # Rubber grip sleeves on handle
    for gy in [-0.18, 0.18]:
        bpy.ops.mesh.primitive_cylinder_add(radius=0.03, depth=0.18, location=(-0.60, gy, 0.96))
        grip = bpy.context.active_object
        grip.rotation_euler = (math.radians(90), 0, 0)
        bpy.ops.object.transform_apply(rotation=True)
        parts.append(grip)

    # 3. Cargo retaining side railings (tubular fencing to keep heavy loot inside)
    # Left rail
    for rx in [-0.25, 0.25]:
        bpy.ops.mesh.primitive_cylinder_add(radius=0.016, depth=0.25, location=(rx, 0.33, 0.35))
        rp = bpy.context.active_object
        parts.append(rp)
    bpy.ops.mesh.primitive_cylinder_add(radius=0.018, depth=1.05, location=(0, 0.33, 0.46))
    rh = bpy.context.active_object
    rh.rotation_euler = (0, math.radians(90), 0)
    bpy.ops.object.transform_apply(rotation=True)
    parts.append(rh)

    # Right rail
    for rx in [-0.25, 0.25]:
        bpy.ops.mesh.primitive_cylinder_add(radius=0.016, depth=0.25, location=(rx, -0.33, 0.35))
        rp = bpy.context.active_object
        parts.append(rp)
    bpy.ops.mesh.primitive_cylinder_add(radius=0.018, depth=1.05, location=(0, -0.33, 0.46))
    rh2 = bpy.context.active_object
    rh2.rotation_euler = (0, math.radians(90), 0)
    bpy.ops.object.transform_apply(rotation=True)
    parts.append(rh2)

    # Front stop rail
    bpy.ops.mesh.primitive_cylinder_add(radius=0.018, depth=0.64, location=(0.60, 0, 0.46))
    f_stop = bpy.context.active_object
    f_stop.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    parts.append(f_stop)

    # 4. Four Heavy-Duty Industrial Caster Wheels
    # 2 Rear Fixed Casters (X = -0.42m) & 2 Front Swivel Casters (X = +0.42m)
    for cx in [-0.42, 0.42]:
        for cy in [-0.25, 0.25]:
            # Caster mounting plate
            bpy.ops.mesh.primitive_cube_add(size=1.0, location=(cx, cy, 0.17))
            c_plate = bpy.context.active_object
            c_plate.scale = (0.12, 0.10, 0.012)
            bpy.ops.object.transform_apply(scale=True)
            parts.append(c_plate)

            # Kingpin / swivel horn
            bpy.ops.mesh.primitive_cylinder_add(radius=0.035, depth=0.03, location=(cx, cy, 0.15))
            swivel = bpy.context.active_object
            parts.append(swivel)

            # Stamped steel dual fork legs
            for fy in [-0.035, 0.035]:
                bpy.ops.mesh.primitive_cube_add(size=1.0, location=(cx, cy + fy, 0.09))
                fork = bpy.context.active_object
                fork.scale = (0.05, 0.008, 0.09)
                bpy.ops.object.transform_apply(scale=True)
                parts.append(fork)

            # Rubber wheel tire (radius 0.085m, contact with ground at Z = 0)
            bpy.ops.mesh.primitive_cylinder_add(radius=0.085, depth=0.05, location=(cx, cy, 0.085))
            wheel = bpy.context.active_object
            wheel.rotation_euler = (math.radians(90), 0, 0)
            bpy.ops.object.transform_apply(rotation=True)
            parts.append(wheel)

            # Wheel center metallic hubcap & axle bolt
            bpy.ops.mesh.primitive_cylinder_add(radius=0.045, depth=0.055, location=(cx, cy, 0.085))
            hub = bpy.context.active_object
            hub.rotation_euler = (math.radians(90), 0, 0)
            bpy.ops.object.transform_apply(rotation=True)
            parts.append(hub)

    # Join visual mesh
    for p in parts:
        p.select_set(True)
    bpy.context.view_layer.objects.active = chassis
    bpy.ops.object.join()
    chassis.name = "SM_Loot_Cart"
    finalize_mesh(chassis, [m_frame, m_deck, m_rubber, m_hardware])

    # Physical Collision: 2 convex boxes (1 bed chassis, 1 push handle)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.26))
    ucx1 = bpy.context.active_object
    ucx1.name = "UCX_SM_Loot_Cart_01"
    ucx1.scale = (1.28, 0.74, 0.48)
    bpy.ops.object.transform_apply(scale=True)

    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(-0.58, 0, 0.65))
    ucx2 = bpy.context.active_object
    ucx2.name = "UCX_SM_Loot_Cart_02"
    ucx2.scale = (0.16, 0.68, 0.65)
    bpy.ops.object.transform_apply(scale=True)

    export_fbx_with_collision(chassis, [ucx1, ucx2], os.path.join(OUTPUT_DIR, "SM_Loot_Cart.fbx"))

# ==============================================================================
# 6. SM_Ceiling_Pipes (Cablages suspendus et tuyauteries de vapeur)
# ==============================================================================
def make_ceiling_pipes():
    reset_scene()
    print("Generating SM_Ceiling_Pipes...")
    
    m_steam = create_mat("Mat_Pipe_SteamLaggingBeige", (0.85, 0.82, 0.74, 1.0), roughness=0.88, metallic=0.05)
    m_steel = create_mat("Mat_Pipe_RustedIron", (0.42, 0.36, 0.32, 1.0), roughness=0.65, metallic=0.7)
    m_valve = create_mat("Mat_Pipe_ValveRed", (0.82, 0.12, 0.10, 1.0), roughness=0.45, metallic=0.4)
    m_cable = create_mat("Mat_Cables_SaggyPVC", (0.08, 0.08, 0.09, 1.0), roughness=0.75, metallic=0.05)
    m_strut = create_mat("Mat_Hanger_Galvanized", (0.65, 0.67, 0.70, 1.0), roughness=0.35, metallic=0.8)
    m_brass = create_mat("Mat_Pipe_BrassDial", (0.80, 0.68, 0.28, 1.0), roughness=0.3, metallic=0.9)

    parts = []

    # Modular span: Length = 4.00m along X (matching 400cm modular ceiling grid)
    # Ceiling is at Z = 0.0m; pipes hang below (Z = -0.28m to -0.65m)

    # 1. Main High-Pressure Steam Pipe (radius 0.10m, running at Y = -0.16m, Z = -0.28m)
    bpy.ops.mesh.primitive_cylinder_add(radius=0.10, depth=4.00, location=(0, -0.16, -0.28))
    steam_pipe = bpy.context.active_object
    steam_pipe.rotation_euler = (0, math.radians(90), 0)
    bpy.ops.object.transform_apply(rotation=True)
    parts.append(steam_pipe)

    # Canvas/Fiberglass thermal insulation lagging sleeves (radius 0.12m)
    for lx in [-1.20, 0.80]:
        bpy.ops.mesh.primitive_cylinder_add(radius=0.12, depth=1.10, location=(lx, -0.16, -0.28))
        lagging = bpy.context.active_object
        lagging.rotation_euler = (0, math.radians(90), 0)
        bpy.ops.object.transform_apply(rotation=True)
        parts.append(lagging)

        # Sheet metal clamping bands on lagging ends
        for bx in [-0.52, 0.52]:
            bpy.ops.mesh.primitive_cylinder_add(radius=0.126, depth=0.035, location=(lx + bx, -0.16, -0.28))
            band = bpy.context.active_object
            band.rotation_euler = (0, math.radians(90), 0)
            bpy.ops.object.transform_apply(rotation=True)
            parts.append(band)

    # Cast iron globe valve with red handwheel (at X = -0.30m)
    bpy.ops.mesh.primitive_cylinder_add(radius=0.15, depth=0.22, location=(-0.30, -0.16, -0.28))
    valve_body = bpy.context.active_object
    valve_body.rotation_euler = (0, math.radians(90), 0)
    bpy.ops.object.transform_apply(rotation=True)
    parts.append(valve_body)

    # Valve bonnet stem rising down/forward
    bpy.ops.mesh.primitive_cylinder_add(radius=0.035, depth=0.16, location=(-0.30, -0.16, -0.42))
    stem = bpy.context.active_object
    parts.append(stem)

    # 5-Spoke Red Handwheel
    bpy.ops.mesh.primitive_torus_add(major_radius=0.14, minor_radius=0.018, location=(-0.30, -0.16, -0.50))
    wheel = bpy.context.active_object
    parts.append(wheel)

    for spoke_ang in range(0, 360, 72):
        s_rad = math.radians(spoke_ang)
        sx = math.cos(s_rad) * 0.07
        sy = math.sin(s_rad) * 0.07
        bpy.ops.mesh.primitive_cylinder_add(radius=0.012, depth=0.13, location=(-0.30 + sx, -0.16 + sy, -0.50))
        spoke = bpy.context.active_object
        spoke.rotation_euler = (math.sin(s_rad), math.cos(s_rad), 0)
        bpy.ops.object.transform_apply(rotation=True)
        parts.append(spoke)

    # Vertical Steam Blowout Nozzle / Tee fitting (at X = +0.20m)
    bpy.ops.mesh.primitive_cylinder_add(radius=0.04, depth=0.18, location=(0.20, -0.16, -0.16))
    vent_tee = bpy.context.active_object
    parts.append(vent_tee)

    # Pressure gauge dial facing downward
    bpy.ops.mesh.primitive_cylinder_add(radius=0.055, depth=0.025, location=(0.20, -0.24, -0.28))
    p_gauge = bpy.context.active_object
    p_gauge.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    parts.append(p_gauge)

    # 2. Secondary Utility / Condensate Pipe (radius 0.05m, running at Y = +0.18m, Z = -0.26m)
    bpy.ops.mesh.primitive_cylinder_add(radius=0.05, depth=4.00, location=(0, 0.18, -0.26))
    pipe2 = bpy.context.active_object
    pipe2.rotation_euler = (0, math.radians(90), 0)
    bpy.ops.object.transform_apply(rotation=True)
    parts.append(pipe2)

    # Flange couplings along pipe2
    for fx in [-1.50, 0.0, 1.50]:
        bpy.ops.mesh.primitive_cylinder_add(radius=0.085, depth=0.04, location=(fx, 0.18, -0.26))
        flange = bpy.context.active_object
        flange.rotation_euler = (0, math.radians(90), 0)
        bpy.ops.object.transform_apply(rotation=True)
        parts.append(flange)

    # 3. Hanging Sagging PVC Cable Bundle (Drooping chain curve underneath)
    # 3 Parallel cables with catenary sag between brackets (at X=-1.30m and X=+1.30m)
    cable_offsets = [
        (-0.02, -0.38),
        (0.04, -0.42),
        (0.00, -0.46)
    ]
    
    # 10 segments per cable to form natural catenary sag
    for coy, coz in cable_offsets:
        seg_count = 12
        span = 4.00
        dx = span / seg_count
        for s in range(seg_count):
            x1 = -2.00 + s * dx
            x2 = x1 + dx
            xm = (x1 + x2) * 0.5
            # Catenary sag parabola formula: sag max at center (X=0)
            sag1 = coz - 0.18 * (1.0 - (x1 / 2.0)**2)
            sag2 = coz - 0.18 * (1.0 - (x2 / 2.0)**2)
            zm = (sag1 + sag2) * 0.5
            dz = sag2 - sag1
            length = math.sqrt(dx*dx + dz*dz)
            
            bpy.ops.mesh.primitive_cylinder_add(radius=0.016, depth=length, location=(xm, coy, zm))
            c_seg = bpy.context.active_object
            angle_y = math.atan2(dz, dx)
            c_seg.rotation_euler = (0, math.radians(90) - angle_y, 0)
            bpy.ops.object.transform_apply(rotation=True)
            parts.append(c_seg)

    # Cable tie bands holding bundle together
    for tx in [-0.80, 0.0, 0.80]:
        bpy.ops.mesh.primitive_torus_add(major_radius=0.055, minor_radius=0.012, location=(tx, 0.01, -0.58))
        tie = bpy.context.active_object
        tie.rotation_euler = (0, math.radians(90), 0)
        bpy.ops.object.transform_apply(rotation=True)
        parts.append(tie)

    # 4. Trapeze Ceiling Suspension Hanger Brackets (at X = -1.30m and X = +1.30m)
    for hx in [-1.30, 1.30]:
        # Horizontal unistrut channel across Y (from Y=-0.38 to +0.38)
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(hx, 0.01, -0.40))
        channel = bpy.context.active_object
        channel.scale = (0.05, 0.82, 0.04)
        bpy.ops.object.transform_apply(scale=True)
        parts.append(channel)

        # U-bolts clamping steam pipe and secondary pipe
        for uy in [-0.16, 0.18]:
            bpy.ops.mesh.primitive_torus_add(major_radius=0.11, minor_radius=0.01, location=(hx, uy, -0.28))
            ubolt = bpy.context.active_object
            ubolt.rotation_euler = (0, math.radians(90), 0)
            bpy.ops.object.transform_apply(rotation=True)
            parts.append(ubolt)

        # 2 Vertical threaded drop rods extending up to ceiling (Z = 0.0m)
        for ry in [-0.36, 0.38]:
            bpy.ops.mesh.primitive_cylinder_add(radius=0.012, depth=0.42, location=(hx, ry, -0.19))
            d_rod = bpy.context.active_object
            parts.append(d_rod)

            # Ceiling mounting plate at Z = 0.0m
            bpy.ops.mesh.primitive_cube_add(size=1.0, location=(hx, ry, 0.005))
            c_plate = bpy.context.active_object
            c_plate.scale = (0.08, 0.08, 0.01)
            bpy.ops.object.transform_apply(scale=True)
            parts.append(c_plate)

    # Join visual mesh
    for p in parts:
        p.select_set(True)
    bpy.context.view_layer.objects.active = steam_pipe
    bpy.ops.object.join()
    steam_pipe.name = "SM_Ceiling_Pipes"
    finalize_mesh(steam_pipe, [m_steam, m_steel, m_valve, m_cable, m_strut, m_brass])

    # Physical Collision: convex box covering pipes and suspension
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0.01, -0.34))
    ucx = bpy.context.active_object
    ucx.name = "UCX_SM_Ceiling_Pipes_01"
    ucx.scale = (4.02, 0.86, 0.68)
    bpy.ops.object.transform_apply(scale=True)

    export_fbx_with_collision(steam_pipe, [ucx], os.path.join(OUTPUT_DIR, "SM_Ceiling_Pipes.fbx"))

# ==============================================================================
# MAIN EXECUTION
# ==============================================================================
if __name__ == "__main__":
    print("=== STARTING GENERATION OF ALL 6 LIMINAL RETRO-INDUSTRIAL PROPS ===")
    make_vending_machine()
    make_vent_duct_straight()
    make_vent_duct_corner()
    make_emergency_breaker()
    make_loot_cart()
    make_ceiling_pipes()
    print("=== ALL 6 ASSETS GENERATED AND EXPORTED SUCCESSFULLY ===")
