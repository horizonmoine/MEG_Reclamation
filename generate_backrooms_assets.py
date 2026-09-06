import bpy
import bmesh
import math
import os
import sys

print('=== STARTING MEG BACKROOMS ASSET GENERATION ===')

OUTPUT_DIR = r'F:\MEG_Reclamation\RawAssets\FBX'
MODULAR_DIR = os.path.join(OUTPUT_DIR, 'Modular')
PROPS_DIR = os.path.join(OUTPUT_DIR, 'Props')
PUZZLES_DIR = os.path.join(OUTPUT_DIR, 'Puzzles')
TOOLS_DIR = os.path.join(OUTPUT_DIR, 'Tools')

for d in [MODULAR_DIR, PROPS_DIR, PUZZLES_DIR, TOOLS_DIR]:
    os.makedirs(d, exist_ok=True)

def reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    if bpy.context.scene.world is None:
        bpy.context.scene.world = bpy.data.worlds.new('World')

def create_mat(name, color, roughness=0.5, metallic=0.0):
    mat = bpy.data.materials.new(name=name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get('Principled BSDF')
    if bsdf:
        bsdf.inputs['Base Color'].default_value = color
        bsdf.inputs['Roughness'].default_value = roughness
        bsdf.inputs['Metallic'].default_value = metallic
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
    
    # Enable auto smooth normals
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
# 1. SM_Wall_Modular_400x300
# ==========================================
def make_modular_wall():
    reset_scene()
    m_wall = create_mat('Mat_Wall_Wallpaper', (0.85, 0.78, 0.55, 1.0), roughness=0.85)
    m_trim = create_mat('Mat_Wall_Trim', (0.35, 0.25, 0.18, 1.0), roughness=0.4)
    
    # Main wall
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 1.5))
    wall = bpy.context.active_object
    wall.scale = (4.0, 0.2, 3.0)
    bpy.ops.object.transform_apply(scale=True)
    
    # Plinthe / Baseboard
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.075))
    base = bpy.context.active_object
    base.scale = (4.0, 0.24, 0.15)
    bpy.ops.object.transform_apply(scale=True)
    
    # Crown molding
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 2.94))
    crown = bpy.context.active_object
    crown.scale = (4.0, 0.25, 0.12)
    bpy.ops.object.transform_apply(scale=True)
    
    # Join into one mesh
    for o in [wall, base, crown]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = wall
    bpy.ops.object.join()
    
    finalize_mesh(wall, [m_wall, m_trim])
    export_fbx(wall, os.path.join(MODULAR_DIR, 'SM_Wall_Modular_400x300.fbx'))

# ==========================================
# 2. SM_Wall_Modular_Doorway_400x300
# ==========================================
def make_doorway_wall():
    reset_scene()
    m_wall = create_mat('Mat_Wall_Wallpaper', (0.85, 0.78, 0.55, 1.0), roughness=0.85)
    m_trim = create_mat('Mat_Wall_Trim', (0.35, 0.25, 0.18, 1.0), roughness=0.4)
    
    # Left pillar: width 1.4m (X: -2.0 to -0.6)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(-1.3, 0, 1.5))
    left = bpy.context.active_object
    left.scale = (1.4, 0.2, 3.0)
    bpy.ops.object.transform_apply(scale=True)
    
    # Right pillar: width 1.4m (X: 0.6 to 2.0)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(1.3, 0, 1.5))
    right = bpy.context.active_object
    right.scale = (1.4, 0.2, 3.0)
    bpy.ops.object.transform_apply(scale=True)
    
    # Lintel above door: width 1.2m, height 0.7m (Z: 2.3 to 3.0)
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 2.65))
    top = bpy.context.active_object
    top.scale = (1.2, 0.2, 0.7)
    bpy.ops.object.transform_apply(scale=True)
    
    # Baseboards on left and right
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(-1.3, 0, 0.075))
    b_left = bpy.context.active_object
    b_left.scale = (1.4, 0.24, 0.15)
    bpy.ops.object.transform_apply(scale=True)
    
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(1.3, 0, 0.075))
    b_right = bpy.context.active_object
    b_right.scale = (1.4, 0.24, 0.15)
    bpy.ops.object.transform_apply(scale=True)
    
    # Crown molding all across top
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 2.94))
    crown = bpy.context.active_object
    crown.scale = (4.0, 0.25, 0.12)
    bpy.ops.object.transform_apply(scale=True)
    
    for o in [left, right, top, b_left, b_right, crown]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = left
    bpy.ops.object.join()
    
    finalize_mesh(left, [m_wall, m_trim])
    export_fbx(left, os.path.join(MODULAR_DIR, 'SM_Wall_Modular_Doorway_400x300.fbx'))

# ==========================================
# 3. SM_Floor_Tile_400x400
# ==========================================
def make_floor_tile():
    reset_scene()
    m_carpet = create_mat('Mat_Floor_Carpet', (0.75, 0.65, 0.40, 1.0), roughness=0.95)
    
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, -0.05))
    floor = bpy.context.active_object
    floor.scale = (4.0, 4.0, 0.1)
    bpy.ops.object.transform_apply(scale=True)
    
    finalize_mesh(floor, [m_carpet])
    export_fbx(floor, os.path.join(MODULAR_DIR, 'SM_Floor_Tile_400x400.fbx'))

# ==========================================
# 4. SM_Ceiling_Tile_400x400
# ==========================================
def make_ceiling_tile():
    reset_scene()
    m_tile = create_mat('Mat_Ceiling_Tile', (0.88, 0.88, 0.85, 1.0), roughness=0.9)
    m_grid = create_mat('Mat_Ceiling_Grid', (0.5, 0.5, 0.52, 1.0), roughness=0.3, metallic=0.7)
    
    # Main ceiling slab
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.025))
    slab = bpy.context.active_object
    slab.scale = (4.0, 4.0, 0.05)
    bpy.ops.object.transform_apply(scale=True)
    
    # Grid runners (T-bars across 4x4 tiles)
    runners = []
    for i in [-1.0, 0.0, 1.0]:
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(i, 0, -0.01))
        rX = bpy.context.active_object
        rX.scale = (0.04, 4.0, 0.02)
        bpy.ops.object.transform_apply(scale=True)
        runners.append(rX)
        
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, i, -0.01))
        rY = bpy.context.active_object
        rY.scale = (4.0, 0.04, 0.02)
        bpy.ops.object.transform_apply(scale=True)
        runners.append(rY)
        
    for r in runners:
        r.select_set(True)
    slab.select_set(True)
    bpy.context.view_layer.objects.active = slab
    bpy.ops.object.join()
    
    finalize_mesh(slab, [m_tile, m_grid])
    export_fbx(slab, os.path.join(MODULAR_DIR, 'SM_Ceiling_Tile_400x400.fbx'))

# ==========================================
# 5. SM_Ceiling_FluorescentLight
# ==========================================
def make_fluorescent_light():
    reset_scene()
    m_metal = create_mat('Mat_Light_MetalHousing', (0.9, 0.9, 0.9, 1.0), roughness=0.3, metallic=0.5)
    m_neon = create_mat('Mat_Light_NeonEmissive', (1.0, 0.98, 0.85, 1.0), roughness=0.1)
    
    # Outer fixture casing
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, -0.05))
    box = bpy.context.active_object
    box.scale = (1.6, 0.45, 0.1)
    bpy.ops.object.transform_apply(scale=True)
    
    # 2 Tubes
    bpy.ops.mesh.primitive_cylinder_add(radius=0.02, depth=1.4, location=(0, -0.1, -0.08))
    tube1 = bpy.context.active_object
    tube1.rotation_euler = (0, math.radians(90), 0)
    bpy.ops.object.transform_apply(rotation=True)
    
    bpy.ops.mesh.primitive_cylinder_add(radius=0.02, depth=1.4, location=(0, 0.1, -0.08))
    tube2 = bpy.context.active_object
    tube2.rotation_euler = (0, math.radians(90), 0)
    bpy.ops.object.transform_apply(rotation=True)
    
    for o in [box, tube1, tube2]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = box
    bpy.ops.object.join()
    
    finalize_mesh(box, [m_metal, m_neon])
    export_fbx(box, os.path.join(MODULAR_DIR, 'SM_Ceiling_FluorescentLight.fbx'))

# ==========================================
# 6. SM_Pillar_Industrial
# ==========================================
def make_pillar():
    reset_scene()
    m_conc = create_mat('Mat_Pillar_Concrete', (0.6, 0.58, 0.55, 1.0), roughness=0.9)
    m_steel = create_mat('Mat_Pillar_MetalGuards', (0.25, 0.25, 0.25, 1.0), roughness=0.4, metallic=0.8)
    
    # Core shaft
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 1.5))
    pillar = bpy.context.active_object
    pillar.scale = (0.7, 0.7, 3.0)
    bpy.ops.object.transform_apply(scale=True)
    
    # Base plinth
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.125))
    base = bpy.context.active_object
    base.scale = (0.85, 0.85, 0.25)
    bpy.ops.object.transform_apply(scale=True)
    
    # Capital top
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 2.9))
    top = bpy.context.active_object
    top.scale = (0.85, 0.85, 0.2)
    bpy.ops.object.transform_apply(scale=True)
    
    for o in [pillar, base, top]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = pillar
    bpy.ops.object.join()
    
    finalize_mesh(pillar, [m_conc, m_steel])
    export_fbx(pillar, os.path.join(MODULAR_DIR, 'SM_Pillar_Industrial.fbx'))

# ==========================================
# 7. SM_Pool_Column
# ==========================================
def make_pool_column():
    reset_scene()
    m_tile = create_mat('Mat_Pool_CeramicTile', (0.92, 0.95, 0.98, 1.0), roughness=0.15)
    
    bpy.ops.mesh.primitive_cylinder_add(radius=0.38, depth=2.7, location=(0, 0, 1.5))
    col = bpy.context.active_object
    
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.075))
    base = bpy.context.active_object
    base.scale = (0.8, 0.8, 0.15)
    bpy.ops.object.transform_apply(scale=True)
    
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 2.925))
    cap = bpy.context.active_object
    cap.scale = (0.8, 0.8, 0.15)
    bpy.ops.object.transform_apply(scale=True)
    
    for o in [col, base, cap]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = col
    bpy.ops.object.join()
    
    finalize_mesh(col, [m_tile])
    export_fbx(col, os.path.join(MODULAR_DIR, 'SM_Pool_Column.fbx'))

# ==========================================
# 8. SM_Industrial_Pipe
# ==========================================
def make_industrial_pipe():
    reset_scene()
    m_pipe = create_mat('Mat_Pipe_RustedSteel', (0.45, 0.35, 0.28, 1.0), roughness=0.7, metallic=0.6)
    m_flange = create_mat('Mat_Pipe_FlangeHardware', (0.2, 0.2, 0.22, 1.0), roughness=0.4, metallic=0.9)
    
    bpy.ops.mesh.primitive_cylinder_add(radius=0.12, depth=4.0, location=(0, 0, 0))
    pipe = bpy.context.active_object
    pipe.rotation_euler = (0, math.radians(90), 0)
    bpy.ops.object.transform_apply(rotation=True)
    
    flanges = []
    for posX in [-1.9, 0.0, 1.9]:
        bpy.ops.mesh.primitive_cylinder_add(radius=0.18, depth=0.06, location=(posX, 0, 0))
        flange = bpy.context.active_object
        flange.rotation_euler = (0, math.radians(90), 0)
        bpy.ops.object.transform_apply(rotation=True)
        flanges.append(flange)
        
    for f in flanges:
        f.select_set(True)
    pipe.select_set(True)
    bpy.context.view_layer.objects.active = pipe
    bpy.ops.object.join()
    
    finalize_mesh(pipe, [m_pipe, m_flange])
    export_fbx(pipe, os.path.join(MODULAR_DIR, 'SM_Industrial_Pipe.fbx'))

# ==========================================
# 9. SM_Door_Frame
# ==========================================
def make_door_frame():
    reset_scene()
    m_metal = create_mat('Mat_DoorFrame_Metal', (0.3, 0.32, 0.35, 1.0), roughness=0.35, metallic=0.7)
    
    # Left post
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(-0.58, 0, 1.175))
    left = bpy.context.active_object
    left.scale = (0.12, 0.22, 2.35)
    bpy.ops.object.transform_apply(scale=True)
    
    # Right post
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.58, 0, 1.175))
    right = bpy.context.active_object
    right.scale = (0.12, 0.22, 2.35)
    bpy.ops.object.transform_apply(scale=True)
    
    # Top header
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 2.3))
    top = bpy.context.active_object
    top.scale = (1.28, 0.22, 0.12)
    bpy.ops.object.transform_apply(scale=True)
    
    for o in [left, right, top]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = left
    bpy.ops.object.join()
    
    finalize_mesh(left, [m_metal])
    export_fbx(left, os.path.join(MODULAR_DIR, 'SM_Door_Frame.fbx'))

# ==========================================
# 10. SM_Door_Leaf
# ==========================================
def make_door_leaf():
    reset_scene()
    m_door = create_mat('Mat_Door_WoodVeneer', (0.5, 0.35, 0.2, 1.0), roughness=0.5)
    m_metal = create_mat('Mat_Door_Hardware', (0.8, 0.8, 0.82, 1.0), roughness=0.2, metallic=0.9)
    m_glass = create_mat('Mat_Door_VisionGlass', (0.8, 0.9, 0.95, 0.4), roughness=0.1)
    
    # Door panel: Pivot at (0,0,0) on hinge edge
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.5, 0, 1.125))
    door = bpy.context.active_object
    door.scale = (1.0, 0.05, 2.25)
    bpy.ops.object.transform_apply(scale=True)
    
    # Kickplate at bottom
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.5, 0, 0.15))
    kick = bpy.context.active_object
    kick.scale = (0.96, 0.056, 0.3)
    bpy.ops.object.transform_apply(scale=True)
    
    # Handle lever
    bpy.ops.mesh.primitive_cylinder_add(radius=0.015, depth=0.14, location=(0.9, 0, 1.0))
    handle_stem = bpy.context.active_object
    handle_stem.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.85, 0.07, 1.0))
    lever = bpy.context.active_object
    lever.scale = (0.12, 0.02, 0.025)
    bpy.ops.object.transform_apply(scale=True)
    
    for o in [door, kick, handle_stem, lever]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = door
    bpy.ops.object.join()
    
    finalize_mesh(door, [m_door, m_metal, m_glass])
    export_fbx(door, os.path.join(MODULAR_DIR, 'SM_Door_Leaf.fbx'))

# ==========================================
# 11. SM_HidingLocker
# ==========================================
def make_hiding_locker():
    reset_scene()
    m_paint = create_mat('Mat_Locker_OliveGreen', (0.28, 0.36, 0.25, 1.0), roughness=0.6)
    m_chrome = create_mat('Mat_Locker_Hardware', (0.85, 0.85, 0.87, 1.0), roughness=0.2, metallic=0.9)
    
    # Locker outer body
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 1.1))
    body = bpy.context.active_object
    body.scale = (0.6, 0.6, 2.2)
    bpy.ops.object.transform_apply(scale=True)
    
    # Door trim & louvers
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0.31, 1.1))
    door = bpy.context.active_object
    door.scale = (0.54, 0.02, 2.14)
    bpy.ops.object.transform_apply(scale=True)
    
    # Latch pocket
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.2, 0.32, 1.1))
    latch = bpy.context.active_object
    latch.scale = (0.06, 0.02, 0.2)
    bpy.ops.object.transform_apply(scale=True)
    
    for o in [body, door, latch]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = body
    bpy.ops.object.join()
    
    finalize_mesh(body, [m_paint, m_chrome])
    export_fbx(body, os.path.join(PROPS_DIR, 'SM_HidingLocker.fbx'))

# ==========================================
# 12. SM_Office_Desk
# ==========================================
def make_office_desk():
    reset_scene()
    m_top = create_mat('Mat_Desk_FauxWoodLaminate', (0.6, 0.45, 0.32, 1.0), roughness=0.4)
    m_metal = create_mat('Mat_Desk_BeigeMetal', (0.7, 0.68, 0.62, 1.0), roughness=0.55)
    
    # Desktop
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.73))
    top = bpy.context.active_object
    top.scale = (1.6, 0.8, 0.04)
    bpy.ops.object.transform_apply(scale=True)
    
    # Left pedestal
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(-0.55, 0, 0.355))
    left_ped = bpy.context.active_object
    left_ped.scale = (0.45, 0.75, 0.71)
    bpy.ops.object.transform_apply(scale=True)
    
    # Right pedestal
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.55, 0, 0.355))
    right_ped = bpy.context.active_object
    right_ped.scale = (0.45, 0.75, 0.71)
    bpy.ops.object.transform_apply(scale=True)
    
    # Back modesty panel
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, -0.36, 0.45))
    back = bpy.context.active_object
    back.scale = (0.7, 0.02, 0.5)
    bpy.ops.object.transform_apply(scale=True)
    
    for o in [top, left_ped, right_ped, back]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = top
    bpy.ops.object.join()
    
    finalize_mesh(top, [m_top, m_metal])
    export_fbx(top, os.path.join(PROPS_DIR, 'SM_Office_Desk.fbx'))

# ==========================================
# 13. SM_Office_Chair
# ==========================================
def make_office_chair():
    reset_scene()
    m_fab = create_mat('Mat_Chair_RoughBrownFabric', (0.35, 0.28, 0.22, 1.0), roughness=0.9)
    m_plast = create_mat('Mat_Chair_BlackPlastic', (0.1, 0.1, 0.12, 1.0), roughness=0.4)
    
    # Seat cushion
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.48))
    seat = bpy.context.active_object
    seat.scale = (0.5, 0.48, 0.08)
    bpy.ops.object.transform_apply(scale=True)
    
    # Backrest
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, -0.22, 0.78))
    back = bpy.context.active_object
    back.scale = (0.44, 0.06, 0.5)
    bpy.ops.object.transform_apply(scale=True)
    
    # Central stem
    bpy.ops.mesh.primitive_cylinder_add(radius=0.035, depth=0.38, location=(0, 0, 0.25))
    stem = bpy.context.active_object
    
    # Base star legs
    legs = []
    for angle in range(0, 360, 72):
        rad = math.radians(angle)
        posX = math.cos(rad) * 0.15
        posY = math.sin(rad) * 0.15
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(posX, posY, 0.06))
        leg = bpy.context.active_object
        leg.scale = (0.3, 0.06, 0.04)
        leg.rotation_euler = (0, 0, rad)
        bpy.ops.object.transform_apply(scale=True, rotation=True)
        legs.append(leg)
        
    for o in [seat, back, stem] + legs:
        o.select_set(True)
    bpy.context.view_layer.objects.active = seat
    bpy.ops.object.join()
    
    finalize_mesh(seat, [m_fab, m_plast])
    export_fbx(seat, os.path.join(PROPS_DIR, 'SM_Office_Chair.fbx'))

# ==========================================
# 14. SM_VentDuct
# ==========================================
def make_vent_duct():
    reset_scene()
    m_galv = create_mat('Mat_Vent_GalvanizedSteel', (0.7, 0.72, 0.75, 1.0), roughness=0.35, metallic=0.8)
    
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.35))
    duct = bpy.context.active_object
    duct.scale = (1.0, 2.0, 0.7)
    bpy.ops.object.transform_apply(scale=True)
    
    # Reinforcing end flanges
    for posY in [-0.98, 0.98]:
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, posY, 0.35))
        flange = bpy.context.active_object
        flange.scale = (1.06, 0.04, 0.76)
        bpy.ops.object.transform_apply(scale=True)
        flange.select_set(True)
        
    duct.select_set(True)
    bpy.context.view_layer.objects.active = duct
    bpy.ops.object.join()
    
    finalize_mesh(duct, [m_galv])
    export_fbx(duct, os.path.join(PROPS_DIR, 'SM_VentDuct.fbx'))

# ==========================================
# 15. SM_Almond_Water_Bottle
# ==========================================
def make_almond_water():
    reset_scene()
    m_glass = create_mat('Mat_Bottle_AmberGlass', (0.6, 0.35, 0.1, 0.85), roughness=0.1)
    m_cap = create_mat('Mat_Bottle_TinCap', (0.8, 0.8, 0.82, 1.0), roughness=0.25, metallic=0.9)
    m_label = create_mat('Mat_Bottle_MEGLabel', (0.9, 0.88, 0.8, 1.0), roughness=0.8)
    
    # Body
    bpy.ops.mesh.primitive_cylinder_add(radius=0.04, depth=0.16, location=(0, 0, 0.08))
    body = bpy.context.active_object
    
    # Neck
    bpy.ops.mesh.primitive_cylinder_add(radius=0.018, depth=0.06, location=(0, 0, 0.19))
    neck = bpy.context.active_object
    
    # Cap
    bpy.ops.mesh.primitive_cylinder_add(radius=0.02, depth=0.025, location=(0, 0, 0.23))
    cap = bpy.context.active_object
    
    # Label band
    bpy.ops.mesh.primitive_cylinder_add(radius=0.041, depth=0.09, location=(0, 0, 0.08))
    label = bpy.context.active_object
    
    for o in [body, neck, cap, label]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = body
    bpy.ops.object.join()
    
    finalize_mesh(body, [m_glass, m_cap, m_label])
    export_fbx(body, os.path.join(PROPS_DIR, 'SM_Almond_Water_Bottle.fbx'))

# ==========================================
# 16. SM_SteamValve
# ==========================================
def make_steam_valve():
    reset_scene()
    m_red = create_mat('Mat_Valve_FireEngineRed', (0.8, 0.1, 0.08, 1.0), roughness=0.4)
    m_brass = create_mat('Mat_Valve_BrassStem', (0.85, 0.7, 0.25, 1.0), roughness=0.3, metallic=0.8)
    
    # Outer ring (Torus)
    bpy.ops.mesh.primitive_torus_add(major_radius=0.16, minor_radius=0.018, location=(0, 0, 0))
    wheel = bpy.context.active_object
    wheel.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    
    # Center hub
    bpy.ops.mesh.primitive_cylinder_add(radius=0.04, depth=0.06, location=(0, 0, 0))
    hub = bpy.context.active_object
    hub.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    
    # 5 Spokes
    spokes = []
    for i in range(5):
        rad = math.radians(i * 72)
        bpy.ops.mesh.primitive_cylinder_add(radius=0.01, depth=0.15, location=(math.cos(rad)*0.08, 0, math.sin(rad)*0.08))
        spoke = bpy.context.active_object
        spoke.rotation_euler = (0, -rad + math.radians(90), 0)
        bpy.ops.object.transform_apply(rotation=True)
        spokes.append(spoke)
        
    # Valve stem
    bpy.ops.mesh.primitive_cylinder_add(radius=0.025, depth=0.18, location=(0, -0.09, 0))
    stem = bpy.context.active_object
    stem.rotation_euler = (math.radians(90), 0, 0)
    bpy.ops.object.transform_apply(rotation=True)
    
    for o in [wheel, hub, stem] + spokes:
        o.select_set(True)
    bpy.context.view_layer.objects.active = wheel
    bpy.ops.object.join()
    
    finalize_mesh(wheel, [m_red, m_brass])
    export_fbx(wheel, os.path.join(PUZZLES_DIR, 'SM_SteamValve.fbx'))

# ==========================================
# 17. SM_FuseBox
# ==========================================
def make_fuse_box():
    reset_scene()
    m_box = create_mat('Mat_FuseBox_IndustrialGray', (0.4, 0.42, 0.45, 1.0), roughness=0.5, metallic=0.5)
    m_parts = create_mat('Mat_FuseBox_CopperCeramic', (0.85, 0.5, 0.2, 1.0), roughness=0.3, metallic=0.7)
    
    # Box body
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.35))
    box = bpy.context.active_object
    box.scale = (0.5, 0.18, 0.7)
    bpy.ops.object.transform_apply(scale=True)
    
    # Door rim & hinge
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0.1, 0.35))
    door = bpy.context.active_object
    door.scale = (0.48, 0.03, 0.68)
    bpy.ops.object.transform_apply(scale=True)
    
    # 3 Fuse cylinders inside/front
    fuses = []
    for i, posX in enumerate([-0.14, 0.0, 0.14]):
        bpy.ops.mesh.primitive_cylinder_add(radius=0.025, depth=0.12, location=(posX, 0.12, 0.35))
        fuse = bpy.context.active_object
        fuses.append(fuse)
        
    for o in [box, door] + fuses:
        o.select_set(True)
    bpy.context.view_layer.objects.active = box
    bpy.ops.object.join()
    
    finalize_mesh(box, [m_box, m_parts])
    export_fbx(box, os.path.join(PUZZLES_DIR, 'SM_FuseBox.fbx'))

# ==========================================
# 18. SM_Keypad
# ==========================================
def make_keypad():
    reset_scene()
    m_body = create_mat('Mat_Keypad_CastAluminum', (0.2, 0.2, 0.22, 1.0), roughness=0.4, metallic=0.6)
    m_keys = create_mat('Mat_Keypad_SiliconeKeys', (0.8, 0.8, 0.8, 1.0), roughness=0.6)
    m_led = create_mat('Mat_Keypad_LEDLens', (0.1, 0.9, 0.1, 1.0), roughness=0.1)
    
    # Housing
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.12))
    pad = bpy.context.active_object
    pad.scale = (0.16, 0.04, 0.24)
    bpy.ops.object.transform_apply(scale=True)
    
    # Buttons 3x4
    buttons = []
    for row in range(4):
        for col in range(3):
            posX = -0.045 + col * 0.045
            posZ = 0.04 + row * 0.035
            bpy.ops.mesh.primitive_cube_add(size=1.0, location=(posX, 0.023, posZ))
            btn = bpy.context.active_object
            btn.scale = (0.028, 0.01, 0.022)
            bpy.ops.object.transform_apply(scale=True)
            buttons.append(btn)
            
    # Top display window
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0.023, 0.19))
    lcd = bpy.context.active_object
    lcd.scale = (0.12, 0.01, 0.035)
    bpy.ops.object.transform_apply(scale=True)
    
    for o in [pad, lcd] + buttons:
        o.select_set(True)
    bpy.context.view_layer.objects.active = pad
    bpy.ops.object.join()
    
    finalize_mesh(pad, [m_body, m_keys, m_led])
    export_fbx(pad, os.path.join(PUZZLES_DIR, 'SM_Keypad.fbx'))

# ==========================================
# 19. SM_Breaker
# ==========================================
def make_breaker():
    reset_scene()
    m_box = create_mat('Mat_Breaker_CabinetGray', (0.35, 0.38, 0.4, 1.0), roughness=0.5, metallic=0.7)
    m_handle = create_mat('Mat_Breaker_HandleRed', (0.85, 0.15, 0.1, 1.0), roughness=0.4)
    
    # Main Box
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.28))
    box = bpy.context.active_object
    box.scale = (0.32, 0.18, 0.55)
    bpy.ops.object.transform_apply(scale=True)
    
    # Lever throw arm on right
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.19, 0, 0.32))
    arm = bpy.context.active_object
    arm.scale = (0.03, 0.05, 0.25)
    arm.rotation_euler = (math.radians(-25), 0, 0)
    bpy.ops.object.transform_apply(scale=True, rotation=True)
    
    # Grip ball
    bpy.ops.mesh.primitive_uv_sphere_add(radius=0.035, location=(0.19, -0.06, 0.42))
    ball = bpy.context.active_object
    
    for o in [box, arm, ball]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = box
    bpy.ops.object.join()
    
    finalize_mesh(box, [m_box, m_handle])
    export_fbx(box, os.path.join(PUZZLES_DIR, 'SM_Breaker.fbx'))

# ==========================================
# 20. SM_Keycard
# ==========================================
def make_keycard():
    reset_scene()
    m_card = create_mat('Mat_Keycard_Polycarbonate', (0.95, 0.95, 0.96, 1.0), roughness=0.3)
    m_chip = create_mat('Mat_Keycard_GoldChip', (0.88, 0.72, 0.2, 1.0), roughness=0.2, metallic=0.9)
    
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.0015))
    card = bpy.context.active_object
    card.scale = (0.086, 0.054, 0.003)
    bpy.ops.object.transform_apply(scale=True)
    
    # Chip contact pad
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(-0.015, 0, 0.0032))
    chip = bpy.context.active_object
    chip.scale = (0.014, 0.012, 0.001)
    bpy.ops.object.transform_apply(scale=True)
    
    for o in [card, chip]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = card
    bpy.ops.object.join()
    
    finalize_mesh(card, [m_card, m_chip])
    export_fbx(card, os.path.join(PUZZLES_DIR, 'SM_Keycard.fbx'))

# ==========================================
# 21. SM_WalkieTalkie
# ==========================================
def make_walkie_talkie():
    reset_scene()
    m_body = create_mat('Mat_Radio_CompositeBlack', (0.12, 0.12, 0.14, 1.0), roughness=0.5)
    m_metal = create_mat('Mat_Radio_DetailsMetal', (0.7, 0.7, 0.72, 1.0), roughness=0.3, metallic=0.8)
    
    # Radio chassis
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, 0.07))
    body = bpy.context.active_object
    body.scale = (0.07, 0.045, 0.14)
    bpy.ops.object.transform_apply(scale=True)
    
    # Antenna
    bpy.ops.mesh.primitive_cylinder_add(radius=0.006, depth=0.12, location=(-0.022, 0, 0.2))
    ant = bpy.context.active_object
    
    # 2 Top knobs
    bpy.ops.mesh.primitive_cylinder_add(radius=0.008, depth=0.02, location=(0.015, 0, 0.15))
    knob1 = bpy.context.active_object
    
    # PTT button on side
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(-0.038, 0, 0.08))
    ptt = bpy.context.active_object
    ptt.scale = (0.01, 0.02, 0.04)
    bpy.ops.object.transform_apply(scale=True)
    
    for o in [body, ant, knob1, ptt]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = body
    bpy.ops.object.join()
    
    finalize_mesh(body, [m_body, m_metal])
    export_fbx(body, os.path.join(TOOLS_DIR, 'SM_WalkieTalkie.fbx'))

# ==========================================
# 22. SM_FlashStrobe
# ==========================================
def make_flash_strobe():
    reset_scene()
    m_body = create_mat('Mat_Flash_AnodizedAluminum', (0.15, 0.15, 0.16, 1.0), roughness=0.35, metallic=0.8)
    m_chrome = create_mat('Mat_Flash_BezelChrome', (0.88, 0.88, 0.9, 1.0), roughness=0.15, metallic=0.95)
    m_lens = create_mat('Mat_Flash_FrontLens', (0.9, 0.95, 1.0, 0.5), roughness=0.05)
    
    # Barrel handle
    bpy.ops.mesh.primitive_cylinder_add(radius=0.022, depth=0.18, location=(0, 0, 0.09))
    handle = bpy.context.active_object
    
    # Expanded head
    bpy.ops.mesh.primitive_cylinder_add(radius=0.048, depth=0.07, location=(0, 0, 0.215))
    head = bpy.context.active_object
    
    # Front lens
    bpy.ops.mesh.primitive_cylinder_add(radius=0.045, depth=0.008, location=(0, 0, 0.252))
    lens = bpy.context.active_object
    
    for o in [handle, head, lens]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = handle
    bpy.ops.object.join()
    
    finalize_mesh(handle, [m_body, m_chrome, m_lens])
    export_fbx(handle, os.path.join(TOOLS_DIR, 'SM_FlashStrobe.fbx'))

# ==========================================
# EXECUTE ALL GENERATORS
# ==========================================
try:
    print('Generating Modular Architecture...')
    make_modular_wall()
    make_doorway_wall()
    make_floor_tile()
    make_ceiling_tile()
    make_fluorescent_light()
    make_pillar()
    make_pool_column()
    make_industrial_pipe()
    make_door_frame()
    make_door_leaf()

    print('Generating Props & Furniture...')
    make_hiding_locker()
    make_office_desk()
    make_office_chair()
    make_vent_duct()
    make_almond_water()

    print('Generating Puzzles & Gameplay Actors...')
    make_steam_valve()
    make_fuse_box()
    make_keypad()
    make_breaker()
    make_keycard()

    print('Generating Tools...')
    make_walkie_talkie()
    make_flash_strobe()

    print('=== ALL 22 BACKROOMS ASSETS GENERATED SUCCESSFULLY ===')
except Exception as e:
    print(f'ERROR DURING ASSET GENERATION: {e}')
    sys.exit(1)
