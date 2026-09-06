import unreal

log = unreal.log

les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
try:
    les.new_level('/Game/Maps/Lvl_ProcGen')
except Exception:
    les.load_level('/Game/Maps/Lvl_ProcGen')

world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()

spawned = []

sun = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 900))
if sun:
    sun.set_actor_rotation(unreal.Rotator(-40, 30, 0), False)
    spawned.append('DirectionalLight')

sky = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 800))
if sky:
    spawned.append('SkyLight')

nav = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(0, 0, 200))
if nav:
    nav.set_actor_scale3d(unreal.Vector(55, 55, 5))
    spawned.append('NavMeshBoundsVolume')

gen_class = unreal.load_object(None, '/Script/MEG_Reclamation.LiminalLevelGenerator')
if gen_class:
    gen = unreal.EditorLevelLibrary.spawn_actor_from_class(gen_class, unreal.Vector(0, 0, 0))
    if gen:
        spawned.append('LiminalLevelGenerator')
else:
    log('BUILD_PROCGEN: classe generatrice introuvable')

ws = world.get_world_settings()
gm_class = unreal.load_object(None, '/Script/MEG_Reclamation.LiminalGameMode')
ws.set_editor_property('default_game_mode', gm_class)

result = les.save_current_level()
log('BUILD_PROCGEN: save_current_level=%s' % result)
if not result:
    result = unreal.EditorLoadingAndSavingUtils.save_map(world, '/Game/Maps/Lvl_ProcGen')
    log('BUILD_PROCGEN: save_map=%s' % result)
if not result:
    result = unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    log('BUILD_PROCGEN: save_dirty=%s' % result)
log('BUILD_PROCGEN: spawns=%s' % spawned)
