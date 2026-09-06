import unreal

log = unreal.log
log("=== MEG : Generation des 11 maps de biomes lancee ===")

les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)

biomes = [
    ("Lvl_00_Lobby", 0),
    ("Lvl_01_HabitableZone", 1),
    ("Lvl_02_PipeDreams", 2),
    ("Lvl_03_ElectricalStation", 3),
    ("Lvl_04_AbandonedOffice", 4),
    ("Lvl_06_LightsOut", 5),
    ("Lvl_08_CaveSystem", 6),
    ("Lvl_09_DarkSuburbs", 7),
    ("Lvl_10_WheatFields", 8),
    ("Lvl_37_Poolrooms", 9),
    ("Lvl_99_RunForYourLife", 10),
]

gen_class = unreal.load_object(None, '/Script/MEG_Reclamation.LiminalLevelGenerator')
gm_class = unreal.load_object(None, '/Script/MEG_Reclamation.LiminalGameMode')

for map_name, biome_id in biomes:
    package_path = f"/Game/Maps/{map_name}"
    log(f"--> Creation de la carte : {package_path} (Biome {biome_id})")
    
    try:
        les.new_level(package_path)
    except Exception as e:
        log(f"Info new_level: {e}")
        try:
            les.load_level(package_path)
        except Exception as e2:
            log(f"Info load_level: {e2}")

    world = ues.get_editor_world()
    if not world:
        log(f"Erreur: Impossible d'obtenir le world pour {map_name}")
        continue

    # Directional Light
    sun = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 1000))
    if sun:
        sun.set_actor_rotation(unreal.Rotator(-45, 35, 0), False)
        sun.set_actor_label(f"{map_name}_Sun")

    # Sky Light
    sky = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 850))
    if sky:
        sky.set_actor_label(f"{map_name}_Sky")

    # NavMesh Bounds Volume
    nav = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(0, 0, 200))
    if nav:
        nav.set_actor_scale3d(unreal.Vector(60, 60, 6))
        nav.set_actor_label(f"{map_name}_NavMesh")

    # LiminalLevelGenerator
    if gen_class:
        gen = unreal.EditorLevelLibrary.spawn_actor_from_class(gen_class, unreal.Vector(0, 0, 0))
        if gen:
            gen.set_actor_label(f"{map_name}_Generator")
            try:
                gen.set_editor_property("biome", biome_id)
            except Exception as ex:
                log(f"Property biome warning: {ex}")

    # Definir le GameMode sur LiminalGameMode
    ws = world.get_world_settings()
    if ws and gm_class:
        ws.set_editor_property('default_game_mode', gm_class)

    # Sauvegarder la carte
    save_ok = unreal.EditorLoadingAndSavingUtils.save_map(world, package_path)
    log(f"Carte {map_name} sauvegardee : {save_ok}")

# Sauvegarder les packages sales
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
log("=== MEG : Les 11 maps de biomes ont ete generees et enregistrees dans /Game/Maps avec succes ! ===")
