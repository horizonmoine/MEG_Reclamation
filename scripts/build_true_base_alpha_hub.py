import unreal

def log(msg):
    unreal.log(f"[HUB_BUILDER] {msg}")

log("==================================================")
log("  M.E.G. : RECONSTRUCTION ARCHITECTURALE DU HUB")
log("  BASE ALPHA — OUTPOST LIMINAIRE COMPACT 12x12m")
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

# 1. Nettoyage integral des anciens acteurs parasites
all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
for a in all_actors:
    try:
        unreal.EditorLevelLibrary.destroy_actor(a)
    except Exception as ex:
        pass
log(f"[OK] Anciens acteurs nettoyes : {len(all_actors)}")

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
mesh_crate = unreal.load_object(None, "/Game/Meshes/Props/SM_SupplyCrate_MEG.SM_SupplyCrate_MEG")
mesh_locker = unreal.load_object(None, "/Game/Meshes/Props/SM_HidingLocker.SM_HidingLocker")
mesh_exit = unreal.load_object(None, "/Game/Meshes/Props/SM_Exit_Sign.SM_Exit_Sign")
mesh_medkit = unreal.load_object(None, "/Game/Meshes/Props/SM_Medkit_MEG.SM_Medkit_MEG")
mesh_scrap_copper = unreal.load_object(None, "/Game/Meshes/Props/SM_Scrap_CopperCable.SM_Scrap_CopperCable")
mesh_scrap_elec = unreal.load_object(None, "/Game/Meshes/Props/SM_Scrap_Electronics.SM_Scrap_Electronics")

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

# 3. Construction de la salle principale (12m x 12m : 3x3 dalles de 400x400)
# X : [-400, 800] -> centres : -200, 200, 600
# Y : [-600, 600] -> centres : -400, 0, 400
room_x = [-200, 200, 600]
room_y = [-400, 0, 400]

# Sol (Floor)
for x in room_x:
    for y in room_y:
        spawn_static_mesh(mesh_floor, unreal.Vector(x, y, 0), label=f"Floor_{x}_{y}")

# Plafond (Ceiling)
for x in room_x:
    for y in room_y:
        spawn_static_mesh(mesh_ceiling, unreal.Vector(x, y, 300), rot=unreal.Rotator(180, 0, 0), label=f"Ceiling_{x}_{y}")

# Murs Sud (Y = -600)
for x in room_x:
    spawn_static_mesh(mesh_wall, unreal.Vector(x, -600, 0), rot=unreal.Rotator(0, 0, 0), label=f"Wall_S_{x}")

# Murs Nord (Y = +600)
for x in room_x:
    spawn_static_mesh(mesh_wall, unreal.Vector(x, 600, 0), rot=unreal.Rotator(0, 180, 0), label=f"Wall_N_{x}")

# Murs Ouest (X = -400)
for y in room_y:
    spawn_static_mesh(mesh_wall, unreal.Vector(-400, y, 0), rot=unreal.Rotator(0, 90, 0), label=f"Wall_W_{y}")

# Murs Est (X = +800) : ouverture centrale pour le couloir du sas
spawn_static_mesh(mesh_wall, unreal.Vector(800, -400, 0), rot=unreal.Rotator(0, -90, 0), label="Wall_E_South")
spawn_static_mesh(mesh_wall, unreal.Vector(800, 400, 0), rot=unreal.Rotator(0, -90, 0), label="Wall_E_North")

# Piliers industriels marquant l'entree du sas
spawn_static_mesh(mesh_pillar, unreal.Vector(800, -200, 0), label="Pillar_Airlock_S")
spawn_static_mesh(mesh_pillar, unreal.Vector(800, 200, 0), label="Pillar_Airlock_N")

# 4. Couloir d'embarquement / Sas d'incursion (4m x 4m a l'Est : X = [800, 1200], Y = [-200, 200])
spawn_static_mesh(mesh_floor, unreal.Vector(1000, 0, 0), label="Airlock_Floor")
spawn_static_mesh(mesh_ceiling, unreal.Vector(1000, 0, 300), rot=unreal.Rotator(180, 0, 0), label="Airlock_Ceiling")
spawn_static_mesh(mesh_wall, unreal.Vector(1000, -200, 0), rot=unreal.Rotator(0, 0, 0), label="Airlock_Wall_S")
spawn_static_mesh(mesh_wall, unreal.Vector(1000, 200, 0), rot=unreal.Rotator(0, 180, 0), label="Airlock_Wall_N")
spawn_static_mesh(mesh_wall, unreal.Vector(1200, 0, 0), rot=unreal.Rotator(0, -90, 0), label="Airlock_Wall_E")

# 5. Sas de decompression M.E.G. (LiminalAirlockActor) & Panneau EXIT
if airlock_class:
    airlock = unreal.EditorLevelLibrary.spawn_actor_from_class(airlock_class, unreal.Vector(1150, 0, 0), unreal.Rotator(0, 180, 0))
    if airlock:
        airlock.set_actor_label("MEG_Mission_Airlock")
        log("[OK] ALiminalAirlockActor instancie a la porte du sas (1150, 0, 0)")

spawn_static_mesh(mesh_exit, unreal.Vector(1100, 0, 240), rot=unreal.Rotator(0, 180, 0), label="ExitSign_Airlock")

# 6. Eclairage diegetique de la Base Alpha
# 4 neons de plafond 60Hz dans la salle principale
main_lights = [
    (0, -300, 285),
    (0, 300, 285),
    (450, -300, 285),
    (450, 300, 285),
]
for idx, (lx, ly, lz) in enumerate(main_lights):
    spawn_static_mesh(mesh_light, unreal.Vector(lx, ly, 295), label=f"CeilingFixture_{idx}")
    pl = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PointLight, unreal.Vector(lx, ly, lz))
    if pl:
        plc = pl.get_editor_property("point_light_component")
        if plc:
            plc.set_editor_property("light_color", unreal.Color(255, 248, 225, 255))
            plc.set_editor_property("intensity", 3800.0)
            plc.set_editor_property("attenuation_radius", 950.0)
        pl.set_actor_label(f"CeilingLight_{idx}")

# Lumiere verte d'ambiance de securite dans le sas
al_light = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PointLight, unreal.Vector(1050, 0, 260))
if al_light:
    alc = al_light.get_editor_property("point_light_component")
    if alc:
        alc.set_editor_property("light_color", unreal.Color(90, 255, 140, 255))
        alc.set_editor_property("intensity", 2400.0)
        alc.set_editor_property("attenuation_radius", 650.0)
    al_light.set_actor_label("Airlock_GreenLight")

# Skylight d'ambiance douce
sky = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(200, 0, 260))
if sky:
    skc = sky.get_editor_property("light_component")
    if skc:
        skc.set_editor_property("intensity", 0.55)
        skc.set_editor_property("light_color", unreal.Color(205, 220, 245, 255))
    sky.set_actor_label("Hub_AmbientSkyLight")

# 7. Bureau de commandement central & Terminal M.E.G.
spawn_static_mesh(mesh_desk, unreal.Vector(140, 0, 0), rot=unreal.Rotator(0, 180, 0), label="Command_Desk")
spawn_static_mesh(mesh_chair, unreal.Vector(220, 0, 0), rot=unreal.Rotator(0, 0, 0), label="Command_Chair")

if terminal_class:
    term = unreal.EditorLevelLibrary.spawn_actor_from_class(terminal_class, unreal.Vector(130, 0, 75), unreal.Rotator(0, 180, 0))
    if term:
        term.set_actor_scale3d(unreal.Vector(0.55, 0.55, 0.55))
        term.set_actor_label("MEG_Interactive_Terminal")
        log("[OK] ALiminalTerminalActor pose et proportionne sur le bureau (130, 0, 75, scale=0.55)")

# 8. Point d'apparition du joueur (PlayerStart)
# Directement en face du bureau et du terminal (a 3.3m), vue plongeante sur l'avant-poste et le sas
pstart = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(-200, 0, 60), unreal.Rotator(0, 0, 0))
if pstart:
    pstart.set_actor_label("Hub_PlayerStart")
    log("[OK] PlayerStart positionne en (-200, 0, 60) face au terminal et au sas")

# 9. Baie logistique & Depot de butin (ExtractionZone)
if extraction_class:
    ext_zone = unreal.EditorLevelLibrary.spawn_actor_from_class(extraction_class, unreal.Vector(200, -450, 20), unreal.Rotator(0, 0, 0))
    if ext_zone:
        ext_zone.set_actor_label("MEG_Scrap_Deposit_Zone")
        log("[OK] AExtractionZone configuree dans la baie logistique (200, -450, 20)")

# Lumiere jaune de dechargement butin
dep_light = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PointLight, unreal.Vector(200, -450, 260))
if dep_light:
    dlc = dep_light.get_editor_property("point_light_component")
    if dlc:
        dlc.set_editor_property("light_color", unreal.Color(255, 200, 60, 255))
        dlc.set_editor_property("intensity", 2200.0)
        dlc.set_editor_property("attenuation_radius", 550.0)
    dep_light.set_actor_label("Deposit_AmberLight")

# Caisses de scrap & composants
spawn_static_mesh(mesh_crate, unreal.Vector(180, -540, 0), rot=unreal.Rotator(0, 15, 0), label="Logistics_Crate_1")
spawn_static_mesh(mesh_crate, unreal.Vector(270, -540, 0), rot=unreal.Rotator(0, -10, 0), label="Logistics_Crate_2")
spawn_static_mesh(mesh_crate, unreal.Vector(225, -540, 72), rot=unreal.Rotator(0, 35, 0), scale=unreal.Vector(0.85, 0.85, 0.85), label="Logistics_Crate_3")
spawn_static_mesh(mesh_scrap_copper, unreal.Vector(120, -520, 10), label="Scrap_Copper_Sample")
spawn_static_mesh(mesh_scrap_elec, unreal.Vector(330, -520, 10), label="Scrap_Elec_Sample")

# 10. Vestiaire tactique M.E.G. (Casiers & Premiers Secours au Nord)
spawn_static_mesh(mesh_locker, unreal.Vector(-100, 560, 0), rot=unreal.Rotator(0, -90, 0), label="Locker_Alpha")
spawn_static_mesh(mesh_locker, unreal.Vector(30, 560, 0), rot=unreal.Rotator(0, -90, 0), label="Locker_Beta")
spawn_static_mesh(mesh_locker, unreal.Vector(160, 560, 0), rot=unreal.Rotator(0, -90, 0), label="Locker_Gamma")

spawn_static_mesh(mesh_crate, unreal.Vector(-100, 440, 0), rot=unreal.Rotator(0, 45, 0), label="SupplyCrate_Med")
spawn_static_mesh(mesh_medkit, unreal.Vector(-100, 440, 75), rot=unreal.Rotator(0, 20, 0), label="Desk_Medkit")

# 11. NavMeshBoundsVolume pour la navigation et physique
nav = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(400, 0, 150))
if nav:
    nav.set_actor_scale3d(unreal.Vector(20, 18, 4))
    nav.set_actor_label("Hub_NavMesh")

# 12. Sauvegarde de la carte Lvl_Hub_BaseAlpha
save_ok = unreal.EditorLoadingAndSavingUtils.save_map(world, map_path)
log(f"[SUCCES] Lvl_Hub_BaseAlpha sauvegardee avec succes : {save_ok}")

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
log("==================================================")
log("  HUB BASE ALPHA GENERE AVEC EXCELLENCE PBR !")
log("==================================================")
