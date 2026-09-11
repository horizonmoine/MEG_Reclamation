"""
Sets up:
1. PhysicsAsset for SK_Hound using SkeletalMeshEditorSubsystem
2. Material assignment on SK_Hound to M_Hound / Material_001
3. AnimMontage AM_Hound_Attack with UAnimNotify_LiminalAttackTrace at apex (0.5s)
4. AnimBlueprint ABP_Hound with SK_Hound_Skeleton
5. Wire BP_Hound CDO and Mesh component
"""

import unreal

DEST_PATH = "/Game/Characters/Bestiary/Hound"

def log(msg):
    print(f"[SETUP_HOUND] {msg}")

def run_setup():
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    skel_subsystem = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)

    sk_hound = unreal.load_asset(f"{DEST_PATH}/SK_Hound")
    skeleton = unreal.load_asset(f"{DEST_PATH}/SK_Hound_Skeleton")
    anim_attack = unreal.load_asset(f"{DEST_PATH}/SK_HoundHound_Attack")
    anim_idle = unreal.load_asset(f"{DEST_PATH}/SK_HoundHound_Idle")
    anim_walk = unreal.load_asset(f"{DEST_PATH}/SK_HoundHound_Walk")
    anim_run = unreal.load_asset(f"{DEST_PATH}/SK_HoundHound_Run")

    if not sk_hound or not skeleton:
        log("ERROR: Missing SK_Hound or Skeleton")
        return False

    # 1. Physics Asset
    log("Setting up PhysicsAsset...")
    pa = sk_hound.get_editor_property("physics_asset")
    if not pa:
        try:
            pa = skel_subsystem.create_physics_asset(sk_hound)
            log(f"PhysicsAsset created via subsystem: {pa}")
        except Exception as e:
            log(f"create_physics_asset error: {e}")

    if pa:
        sk_hound.set_editor_property("physics_asset", pa)
        unreal.EditorAssetLibrary.save_loaded_asset(pa)
        log("PhysicsAsset linked and saved.")

    # 2. Material setup
    log("Setting up Materials...")
    mat = unreal.load_asset("/Game/Meshes/Hound/Material_001")
    if mat:
        materials = sk_hound.get_editor_property("materials")
        if len(materials) > 0:
            materials[0].material_interface = mat
            sk_hound.set_editor_property("materials", materials)
            log("Material_001 assigned to SK_Hound.")

    # Save SK_Hound
    unreal.EditorAssetLibrary.save_loaded_asset(sk_hound)

    # 3. AnimMontage AM_Hound_Attack
    log("Setting up AnimMontage AM_Hound_Attack...")
    am_path = f"{DEST_PATH}/AM_Hound_Attack"
    am = unreal.load_asset(am_path)
    if not am:
        montage_factory = unreal.AnimMontageFactory()
        montage_factory.set_editor_property("target_skeleton", skeleton)
        montage_factory.set_editor_property("source_animation", anim_attack)
        am = asset_tools.create_asset("AM_Hound_Attack", DEST_PATH, unreal.AnimMontage, montage_factory)
        log(f"Created AM_Hound_Attack: {am}")

    if am:
        # Check notifies
        notify_class = unreal.load_class(None, "/Script/MEG_Reclamation.AnimNotify_LiminalAttackTrace")
        if notify_class:
            # Clear existing notifies on track 1 first to avoid duplicates
            unreal.AnimationLibrary.remove_all_animation_notify_tracks(am)
            unreal.AnimationLibrary.add_animation_notify_track(am, "AttackTrack")
            
            notify_inst = unreal.new_object(notify_class, am)
            notify_inst.set_editor_property("SocketName", "AttackSocket")
            notify_inst.set_editor_property("TraceRadius", 45.0)
            notify_inst.set_editor_property("TraceDistance", 100.0)
            
            unreal.AnimationLibrary.add_animation_notify_event_object(am, 0.5, notify_inst, "AttackTrack")
            log("Added AnimNotify_LiminalAttackTrace to AM_Hound_Attack at 0.5s.")

        unreal.EditorAssetLibrary.save_loaded_asset(am)

    # 4. Animation Blueprint ABP_Hound
    log("Setting up AnimBlueprint ABP_Hound...")
    abp_path = f"{DEST_PATH}/ABP_Hound"
    abp = unreal.load_asset(abp_path)
    if not abp:
        abp_factory = unreal.AnimBlueprintFactory()
        abp_factory.set_editor_property("target_skeleton", skeleton)
        abp_factory.set_editor_property("parent_class", unreal.AnimInstance)
        abp_factory.set_editor_property("preview_skeletal_mesh", sk_hound)
        abp = asset_tools.create_asset("ABP_Hound", DEST_PATH, unreal.AnimBlueprint, abp_factory)
        log(f"Created ABP_Hound: {abp}")

    if abp:
        unreal.EditorAssetLibrary.save_loaded_asset(abp)

    # 5. Wire BP_Hound
    log("Wiring BP_Hound...")
    bp_hound = unreal.load_object(None, "/Game/AI/BP_Hound.BP_Hound")
    if bp_hound:
        gc = bp_hound.generated_class()
        cdo = unreal.get_default_object(gc)

        if hasattr(cdo, "DefaultSkeletalMesh"):
            cdo.set_editor_property("DefaultSkeletalMesh", sk_hound)
            log("Set DefaultSkeletalMesh on CDO.")

        if hasattr(cdo, "AttackMontage") and am:
            cdo.set_editor_property("AttackMontage", am)
            log("Set AttackMontage on CDO.")

        abp_class = abp.generated_class() if abp else None
        if hasattr(cdo, "DefaultAnimClass") and abp_class:
            cdo.set_editor_property("DefaultAnimClass", abp_class)
            log("Set DefaultAnimClass on CDO.")

        mesh_comp = cdo.get_editor_property("Mesh")
        if mesh_comp:
            mesh_comp.set_editor_property("skeletal_mesh_asset", sk_hound)
            if abp_class:
                mesh_comp.set_editor_property("anim_class", abp_class)
            log("Wired Mesh component with SK_Hound and ABP_Hound.")

        body_mesh = cdo.get_editor_property("BodyMesh")
        if body_mesh:
            body_mesh.set_editor_property("visible", False)
            log("BodyMesh static mesh fallback hidden.")

        unreal.BlueprintEditorLibrary.compile_blueprint(bp_hound)
        unreal.EditorAssetLibrary.save_loaded_asset(bp_hound)
        log("BP_Hound compiled and saved.")

    return True

if __name__ == "__main__":
    ok = run_setup()
    log(f"Setup finished with result={ok}")
