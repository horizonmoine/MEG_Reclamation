import unreal

def log(msg):
    unreal.log(f"[HUB_BUILDER] {msg}")
    print(f"[HUB_BUILDER] {msg}")

log("==================================================")
log("  M.E.G. : FINALISATION QUALITE DU HUB BASE ALPHA")
log("  MISSION 07 — REGLES DE ZONE ET SECURITE")
log("==================================================")

les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)

map_path = "/Game/Maps/Lvl_Hub_BaseAlpha"
log(f"--> Chargement de la carte : {map_path}")

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
safe_zone_class = unreal.load_object(None, '/Script/MEG_Reclamation.LiminalSafeZoneVolume')
fuse_box_class = unreal.load_object(None, '/Script/MEG_Reclamation.LiminalFuseBoxActor')
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

# 3. Salle principale Base Alpha (12m x 12m)
room_x = [-200, 200, 600]
room_y = [-400, 0, 400]

# Sol
for x in room_x:
    for y in room_y:
        spawn_static_mesh(mesh_floor, unreal.Vector(x, y, 0), label=f"Floor_{x}_{y}")

# Plafond
for x in room_x:
    for y in room_y:
        spawn_static_mesh(mesh_ceiling, unreal.Vector(x, y, 300), rot=unreal.Rotator(180, 0, 0), label=f"Ceiling_{x}_{y}")

# Murs Sud (Y = -600)
for x in room_x:
    spawn_static_mesh(mesh_wall, unreal.Vector(x, -600, 0), rot=unreal.Rotator(0, 0, 0), label=f"Wall_S_{x}")

# Murs Nord (Y = +600)
for x in room_x:
    spawn_static_mesh(mesh_wall, unreal.Vector(x, 600, 0), rot=unreal.Rotator(0, 180, 0), label=f"Wall_N_{x}")

# Murs Ouest (X = -400) - Enceinte etanche
for y in room_y:
    spawn_static_mesh(mesh_wall, unreal.Vector(-400, y, 0), rot=unreal.Rotator(0, 90, 0), label=f"Wall_W_{y}")

# Murs Est (X = +800) avec ouverture centrale vers le sas
spawn_static_mesh(mesh_wall, unreal.Vector(800, -400, 0), rot=unreal.Rotator(0, -90, 0), label="Wall_E_South")
spawn_static_mesh(mesh_wall, unreal.Vector(800, 400, 0), rot=unreal.Rotator(0, -90, 0), label="Wall_E_North")

# Piliers d'entree du sas
spawn_static_mesh(mesh_pillar, unreal.Vector(800, -200, 0), label="Pillar_Airlock_S")
spawn_static_mesh(mesh_pillar, unreal.Vector(800, 200, 0), label="Pillar_Airlock_N")

# 4. Couloir d'embarquement et sas B.R.C. (X = [800, 1200], Y = [-200, 200])
spawn_static_mesh(mesh_floor, unreal.Vector(1000, 0, 0), label="Airlock_Floor")
spawn_static_mesh(mesh_ceiling, unreal.Vector(1000, 0, 300), rot=unreal.Rotator(180, 0, 0), label="Airlock_Ceiling")
spawn_static_mesh(mesh_wall, unreal.Vector(1000, -200, 0), rot=unreal.Rotator(0, 0, 0), label="Airlock_Wall_S")
spawn_static_mesh(mesh_wall, unreal.Vector(1000, 200, 0), rot=unreal.Rotator(0, 180, 0), label="Airlock_Wall_N")
spawn_static_mesh(mesh_wall, unreal.Vector(1200, 0, 0), rot=unreal.Rotator(0, -90, 0), label="Airlock_Wall_E")

# 5. Sas d'incursion (ALiminalAirlockActor) & Panneau EXIT
if airlock_class:
    airlock = unreal.EditorLevelLibrary.spawn_actor_from_class(airlock_class, unreal.Vector(1150, 0, 0), unreal.Rotator(0, 180, 0))
    if airlock:
        airlock.set_actor_label("MEG_Mission_Airlock")
        log("[OK] ALiminalAirlockActor instancie a la porte du sas (1150, 0, 0)")

spawn_static_mesh(mesh_exit, unreal.Vector(1100, 0, 240), rot=unreal.Rotator(0, 180, 0), label="ExitSign_Airlock")

# 6. SafeZoneVolumes
if safe_zone_class:
    # 6.1 Volume global englobant toute la Base Alpha (extent 2500 x 2000 x 500, SanityRestore = 2.0)
    sz_global = unreal.EditorLevelLibrary.spawn_actor_from_class(safe_zone_class, unreal.Vector(400, 0, 200))
    if sz_global:
        sz_global.set_box_extent(unreal.Vector(2500, 2000, 500))
        sz_global.set_sanity_restore_per_second(2.0)
        sz_global.set_actor_label("Global_SafeZone_BaseAlpha")
        log("[OK] ALiminalSafeZoneVolume global instancie (2500x2000x500, Restore=2.0)")

    # 6.2 Second volume sur l'infirmerie (SanityRestore = 8.0)
    sz_med = unreal.EditorLevelLibrary.spawn_actor_from_class(safe_zone_class, unreal.Vector(-100, 480, 100))
    if sz_med:
        sz_med.set_box_extent(unreal.Vector(350, 250, 200))
        sz_med.set_sanity_restore_per_second(8.0)
        sz_med.set_actor_label("Infirmary_SafeZone_BaseAlpha")
        log("[OK] ALiminalSafeZoneVolume infirmerie instancie (350x250x200, Restore=8.0)")

# 7. Eclairage stable 4000 K (blanc neutre chaud) sans aucun ULiminalFlickerLightComponent
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
            plc.set_editor_property("light_color", unreal.Color(255, 238, 215, 255)) # 4000K
            plc.set_editor_property("intensity", 3600.0)
            plc.set_editor_property("attenuation_radius", 950.0)
        pl.set_actor_label(f"CeilingLight_4000K_{idx}")

# Lumiere de sas
al_light = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PointLight, unreal.Vector(1050, 0, 260))
if al_light:
    alc = al_light.get_editor_property("point_light_component")
    if alc:
        alc.set_editor_property("light_color", unreal.Color(90, 255, 140, 255))
        alc.set_editor_property("intensity", 2400.0)
        alc.set_editor_property("attenuation_radius", 650.0)
    al_light.set_actor_label("Airlock_GreenLight")

# Skylight douce
sky = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(200, 0, 260))
if sky:
    skc = sky.get_editor_property("light_component")
    if skc:
        skc.set_editor_property("intensity", 0.50)
        skc.set_editor_property("light_color", unreal.Color(220, 230, 245, 255))
    sky.set_actor_label("Hub_AmbientSkyLight")

# 8. Bureau de commandement central & Terminal M.E.G.
spawn_static_mesh(mesh_desk, unreal.Vector(140, 0, 0), rot=unreal.Rotator(0, 180, 0), label="Command_Desk")
spawn_static_mesh(mesh_chair, unreal.Vector(220, 0, 0), rot=unreal.Rotator(0, 0, 0), label="Command_Chair")

if terminal_class:
    term = unreal.EditorLevelLibrary.spawn_actor_from_class(terminal_class, unreal.Vector(130, 0, 75), unreal.Rotator(0, 180, 0))
    if term:
        term.set_actor_scale3d(unreal.Vector(0.55, 0.55, 0.55))
        term.set_actor_label("MEG_Interactive_Terminal")
        log("[OK] ALiminalTerminalActor instancie sur le bureau (130, 0, 75, scale=0.55)")

# 9. 4 PlayerStart pour l'escouade M.E.G.
pstart_locs = [
    (-200, -150, 60),
    (-200, -50, 60),
    (-200, 50, 60),
    (-200, 150, 60),
]
for idx, (px, py, pz) in enumerate(pstart_locs):
    ps = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(px, py, pz), unreal.Rotator(0, 0, 0))
    if ps:
        ps.set_actor_label(f"Hub_PlayerStart_{idx}")
log("[OK] 4 PlayerStart instancies pour l'escouade")

# 10. Baie logistique & Zone de depot de butin
if extraction_class:
    ext_zone = unreal.EditorLevelLibrary.spawn_actor_from_class(extraction_class, unreal.Vector(200, -450, 20), unreal.Rotator(0, 0, 0))
    if ext_zone:
        ext_zone.set_actor_label("MEG_Scrap_Deposit_Zone")
        log("[OK] AExtractionZone configuree dans la baie logistique (200, -450, 20)")

dep_light = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PointLight, unreal.Vector(200, -450, 260))
if dep_light:
    dlc = dep_light.get_editor_property("point_light_component")
    if dlc:
        dlc.set_editor_property("light_color", unreal.Color(255, 210, 80, 255))
        dlc.set_editor_property("intensity", 2200.0)
        dlc.set_editor_property("attenuation_radius", 550.0)
    dep_light.set_actor_label("Deposit_AmberLight")

spawn_static_mesh(mesh_crate, unreal.Vector(180, -540, 0), rot=unreal.Rotator(0, 15, 0), label="Logistics_Crate_1")
spawn_static_mesh(mesh_crate, unreal.Vector(270, -540, 0), rot=unreal.Rotator(0, -10, 0), label="Logistics_Crate_2")
spawn_static_mesh(mesh_crate, unreal.Vector(225, -540, 72), rot=unreal.Rotator(0, 35, 0), scale=unreal.Vector(0.85, 0.85, 0.85), label="Logistics_Crate_3")
spawn_static_mesh(mesh_scrap_copper, unreal.Vector(120, -520, 10), label="Scrap_Copper_Sample")
spawn_static_mesh(mesh_scrap_elec, unreal.Vector(330, -520, 10), label="Scrap_Elec_Sample")

# 11. Vestiaire et Infirmerie M.E.G. (Zone de soin mental +8 sanite/s)
spawn_static_mesh(mesh_locker, unreal.Vector(-100, 560, 0), rot=unreal.Rotator(0, -90, 0), label="Locker_Alpha")
spawn_static_mesh(mesh_locker, unreal.Vector(30, 560, 0), rot=unreal.Rotator(0, -90, 0), label="Locker_Beta")
spawn_static_mesh(mesh_locker, unreal.Vector(160, 560, 0), rot=unreal.Rotator(0, -90, 0), label="Locker_Gamma")

spawn_static_mesh(mesh_crate, unreal.Vector(-100, 440, 0), rot=unreal.Rotator(0, 45, 0), label="SupplyCrate_Med")
spawn_static_mesh(mesh_medkit, unreal.Vector(-100, 440, 75), rot=unreal.Rotator(0, 20, 0), label="Desk_Medkit")

# 12. FuseBox tutoriel sur le mur Ouest
if fuse_box_class:
    fuse = unreal.EditorLevelLibrary.spawn_actor_from_class(fuse_box_class, unreal.Vector(-380, -250, 120), unreal.Rotator(0, 0, 0))
    if fuse:
        fuse.set_actor_label("Tutorial_FuseBox")
        log("[OK] ALiminalFuseBoxActor tutoriel instancie sur le mur Ouest (-380, -250, 120)")

# 13. NavMeshBoundsVolume
nav = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(400, 0, 150))
if nav:
    nav.set_actor_scale3d(unreal.Vector(20, 18, 4))
    nav.set_actor_label("Hub_NavMesh")

# 14. Sauvegarde
save_ok = unreal.EditorLoadingAndSavingUtils.save_map(world, map_path)
log(f"[SUCCES] Lvl_Hub_BaseAlpha sauvegardee avec succes : {save_ok}")

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
log("==================================================")
log("  HUB BASE ALPHA CONFIGURE AVEC SUCCES !")
log("==================================================")
