import unreal

log = unreal.log
log("==================================================")
log("  M.E.G. : RECONSTRUCTION DU HUB BASE ALPHA")
log("==================================================")

les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)

map_path = "/Game/Maps/Lvl_Hub_BaseAlpha"
log(f"--> Chargement / Creation de la carte : {map_path}")

try:
    les.load_level(map_path)
except Exception as e:
    log(f"Level load info: {e}")
    les.new_level(map_path)

world = ues.get_editor_world()
if not world:
    raise RuntimeError("Impossible d'obtenir le World pour Lvl_Hub_BaseAlpha")

# 1. Nettoyage integral des anciens acteurs parasites (cubes, monstres erratiques)
all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
for a in all_actors:
    try:
        unreal.EditorLevelLibrary.destroy_actor(a)
    except Exception as ex:
        pass
log(f"Anciens acteurs nettoyes : {len(all_actors)}")

# 2. Configuration du GameMode sur LiminalLobbyGameMode
gm_class = unreal.load_object(None, '/Script/MEG_Reclamation.LiminalLobbyGameMode')
ws = world.get_world_settings()
if ws and gm_class:
    ws.set_editor_property('default_game_mode', gm_class)
    log("[OK] Default GameMode configure sur LiminalLobbyGameMode")

# Chargement des classes de gameplay M.E.G.
terminal_class = unreal.load_object(None, '/Script/MEG_Reclamation.LiminalTerminalActor')
airlock_class = unreal.load_object(None, '/Script/MEG_Reclamation.LiminalAirlockActor')
extraction_class = unreal.load_object(None, '/Script/MEG_Reclamation.ExtractionZone')

# Chargement des maillages modulaires et props PBR
mesh_floor = unreal.load_object(None, "/Game/Meshes/Modular/SM_Floor_Tile_400x400.SM_Floor_Tile_400x400")
mesh_wall = unreal.load_object(None, "/Game/Meshes/Modular/SM_Wall_Modular_400x300.SM_Wall_Modular_400x300")
mesh_ceiling = unreal.load_object(None, "/Game/Meshes/Modular/SM_Ceiling_Tile_400x400.SM_Ceiling_Tile_400x400")
mesh_pillar = unreal.load_object(None, "/Game/Meshes/Modular/SM_Pillar_Industrial.SM_Pillar_Industrial")
mesh_light = unreal.load_object(None, "/Game/Meshes/Modular/SM_Ceiling_FluorescentLight.SM_Ceiling_FluorescentLight")

mesh_desk = unreal.load_object(None, "/Game/Meshes/Props/SM_Office_Desk.SM_Office_Desk")
mesh_chair = unreal.load_object(None, "/Game/Meshes/Props/SM_Office_Chair.SM_Office_Chair")
mesh_terminal_prop = unreal.load_object(None, "/Game/Meshes/Props/SM_Terminal_MEG.SM_Terminal_MEG")
mesh_retro_pc = unreal.load_object(None, "/Game/Meshes/Props/SM_Retro_Computer.SM_Retro_Computer")
mesh_crate = unreal.load_object(None, "/Game/Meshes/Props/SM_SupplyCrate_MEG.SM_SupplyCrate_MEG")
mesh_locker = unreal.load_object(None, "/Game/Meshes/Props/SM_HidingLocker.SM_HidingLocker")
mesh_exit = unreal.load_object(None, "/Game/Meshes/Props/SM_Exit_Sign.SM_Exit_Sign")
mesh_vending = unreal.load_object(None, "/Game/Meshes/Props/SM_Vending_Machine.SM_Vending_Machine")
mesh_cart = unreal.load_object(None, "/Game/Meshes/Props/SM_Loot_Cart.SM_Loot_Cart")

def spawn_static_mesh(mesh, loc, rot=unreal.Rotator(0,0,0), scale=unreal.Vector(1,1,1), label="Prop"):
    if not mesh:
        return None
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor, loc, rot)
    if actor:
        smc = actor.get_editor_property("static_mesh_component")
        if smc:
            smc.set_editor_property("static_mesh", mesh)
            smc.set_world_scale3d(scale)
        actor.set_actor_label(label)
    return actor

# 3. Construction de l'architecture de la Base Alpha
x_coords = [-600, -200, 200, 600, 1000, 1400]
y_coords = [-1200, -800, -400, 0, 400, 800, 1200]

# Sol (Floor)
for x in x_coords[:-1]:
    for y in y_coords[:-1]:
        spawn_static_mesh(mesh_floor, unreal.Vector(x + 200, y + 200, 0), label=f"Floor_{x}_{y}")

# Plafond (Ceiling)
for x in x_coords[:-1]:
    for y in y_coords[:-1]:
        spawn_static_mesh(mesh_ceiling, unreal.Vector(x + 200, y + 200, 300), rot=unreal.Rotator(180, 0, 0), label=f"Ceiling_{x}_{y}")

# Murs Nord (Y = 1200) et Sud (Y = -1200)
for x in x_coords[:-1]:
    spawn_static_mesh(mesh_wall, unreal.Vector(x + 200, 1200, 0), rot=unreal.Rotator(0, 180, 0), label=f"Wall_N_{x}")
    spawn_static_mesh(mesh_wall, unreal.Vector(x + 200, -1200, 0), rot=unreal.Rotator(0, 0, 0), label=f"Wall_S_{x}")

# Murs Est (X = 1400) et Ouest (X = -600)
for y in y_coords[:-1]:
    spawn_static_mesh(mesh_wall, unreal.Vector(1400, y + 200, 0), rot=unreal.Rotator(0, -90, 0), label=f"Wall_E_{y}")
    spawn_static_mesh(mesh_wall, unreal.Vector(-600, y + 200, 0), rot=unreal.Rotator(0, 90, 0), label=f"Wall_W_{y}")

# Piliers industriels de renfort
for px, py in [(-200, -600), (-200, 600), (1000, -600), (1000, 600)]:
    spawn_static_mesh(mesh_pillar, unreal.Vector(px, py, 0), label=f"Pillar_{px}_{py}")

# 4. Eclairage diegetique aux neons fluorescents 60Hz
light_locs = [
    (0, -400, 285),
    (0, 400, 285),
    (800, -400, 285),
    (800, 400, 285),
    (400, 0, 285)
]
for idx, (lx, ly, lz) in enumerate(light_locs):
    spawn_static_mesh(mesh_light, unreal.Vector(lx, ly, 295), label=f"NeonFixture_{idx}")
    pl = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PointLight, unreal.Vector(lx, ly, lz))
    if pl:
        plc = pl.get_editor_property("point_light_component")
        if plc:
            plc.set_editor_property("light_color", unreal.Color(255, 245, 220, 255))
            plc.set_editor_property("intensity", 4500.0)
            plc.set_editor_property("attenuation_radius", 1400.0)
        pl.set_actor_label(f"NeonLight_{idx}")

sky = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(400, 0, 260))
if sky:
    skc = sky.get_editor_property("light_component")
    if skc:
        skc.set_editor_property("intensity", 0.6)
        skc.set_editor_property("light_color", unreal.Color(200, 220, 240, 255))
    sky.set_actor_label("Hub_AmbientSkyLight")

# 5. Bureau de commandement, Terminal M.E.G. et Briefing
spawn_static_mesh(mesh_desk, unreal.Vector(250, 0, 0), rot=unreal.Rotator(0, -90, 0), label="Command_Desk")
spawn_static_mesh(mesh_chair, unreal.Vector(330, 0, 0), rot=unreal.Rotator(0, 90, 0), label="Command_Chair")
spawn_static_mesh(mesh_retro_pc, unreal.Vector(240, -50, 75), rot=unreal.Rotator(0, -90, 0), label="Desk_PC_Monitor")

# Terminal PC interactif M.E.G.
if terminal_class:
    term = unreal.EditorLevelLibrary.spawn_actor_from_class(terminal_class, unreal.Vector(240, 50, 75), unreal.Rotator(0, 180, 0))
    if term:
        term.set_actor_label("MEG_Interactive_Terminal")
        log("[OK] ALiminalTerminalActor instancie sur le bureau de commandement")

# 6. Point de spawn du joueur (PlayerStart)
pstart = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(-150, 0, 100), unreal.Rotator(0, 0, 0))
if pstart:
    pstart.set_actor_label("Hub_PlayerStart")
    log("[OK] PlayerStart positionne en face du bureau et du terminal")

# 7. Sas d'embarquement / Excursion (Airlock)
if airlock_class:
    airlock = unreal.EditorLevelLibrary.spawn_actor_from_class(airlock_class, unreal.Vector(1250, 0, 0), unreal.Rotator(0, 180, 0))
    if airlock:
        airlock.set_actor_label("MEG_Mission_Airlock")
        log("[OK] ALiminalAirlockActor instancie a l'entree du sas de deploiement")

spawn_static_mesh(mesh_exit, unreal.Vector(1200, 0, 240), rot=unreal.Rotator(0, 180, 0), label="ExitSign_Airlock")

# 8. Zone de depot de butin / Extraction
if extraction_class:
    ext_zone = unreal.EditorLevelLibrary.spawn_actor_from_class(extraction_class, unreal.Vector(-400, -800, 50), unreal.Rotator(0, 0, 0))
    if ext_zone:
        ext_zone.set_actor_label("MEG_Scrap_Deposit_Zone")
        log("[OK] AExtractionZone instanciee dans la baie de depot logistique")

# 9. Mobiliers logistiques d'ambiance
spawn_static_mesh(mesh_crate, unreal.Vector(-400, -950, 0), rot=unreal.Rotator(0, 20, 0), label="SupplyCrate_1")
spawn_static_mesh(mesh_crate, unreal.Vector(-300, -950, 0), rot=unreal.Rotator(0, -15, 0), label="SupplyCrate_2")
spawn_static_mesh(mesh_crate, unreal.Vector(-350, -950, 75), rot=unreal.Rotator(0, 45, 0), scale=unreal.Vector(0.9, 0.9, 0.9), label="SupplyCrate_3")

spawn_static_mesh(mesh_locker, unreal.Vector(-550, 300, 0), rot=unreal.Rotator(0, 90, 0), label="Locker_1")
spawn_static_mesh(mesh_locker, unreal.Vector(-550, 420, 0), rot=unreal.Rotator(0, 90, 0), label="Locker_2")
spawn_static_mesh(mesh_locker, unreal.Vector(-550, 540, 0), rot=unreal.Rotator(0, 90, 0), label="Locker_3")

spawn_static_mesh(mesh_vending, unreal.Vector(-550, -400, 0), rot=unreal.Rotator(0, 90, 0), label="VendingMachine_AlmondWater")
spawn_static_mesh(mesh_cart, unreal.Vector(600, 800, 0), rot=unreal.Rotator(0, -45, 0), label="LootCart_Transport")

# 10. NavMeshBoundsVolume pour la navigation et collisions
nav = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(400, 0, 150))
if nav:
    nav.set_actor_scale3d(unreal.Vector(25, 30, 4))
    nav.set_actor_label("Hub_NavMesh")

# 11. Sauvegarde de la carte Lvl_Hub_BaseAlpha
save_ok = unreal.EditorLoadingAndSavingUtils.save_map(world, map_path)
log(f"[SUCCES] Lvl_Hub_BaseAlpha sauvegardee avec succes : {save_ok}")

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
log("==================================================")
log("  HUB BASE ALPHA GENERE AVEC EXCELLENCE PBR !")
log("==================================================")
