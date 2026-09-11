import unreal

def wire():
    bp_hound = unreal.load_object(None, "/Game/AI/BP_Hound.BP_Hound")
    sk_hound = unreal.load_asset("/Game/Characters/Bestiary/Hound/SK_Hound")
    abp_hound = unreal.load_asset("/Game/Characters/Bestiary/Hound/ABP_Hound")
    am_attack = unreal.load_asset("/Game/Characters/Bestiary/Hound/AM_Hound_Attack")

    print(f"BP_Hound: {bp_hound}")
    print(f"SK_Hound: {sk_hound}")
    print(f"ABP_Hound: {abp_hound}")
    print(f"AM_Attack: {am_attack}")

    gc = bp_hound.generated_class()
    cdo = unreal.get_default_object(gc)

    # CDO properties
    cdo.set_editor_property("DefaultSkeletalMesh", sk_hound)
    cdo.set_editor_property("DefaultAnimClass", abp_hound.generated_class())
    cdo.set_editor_property("AttackMontage", am_attack)
    cdo.set_editor_property("DefaultAttackSocket", unreal.Name("AttackSocket"))

    # Component properties
    mesh = cdo.get_editor_property("Mesh")
    if mesh:
        mesh.set_editor_property("skeletal_mesh_asset", sk_hound)
        mesh.set_editor_property("anim_class", abp_hound.generated_class())
        mesh.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, -90.0))
        mesh.set_editor_property("relative_rotation", unreal.Rotator(0.0, -90.0, 0.0))
        print("Wired Mesh component.")

    body_mesh = cdo.get_editor_property("BodyMesh")
    if body_mesh:
        body_mesh.set_editor_property("static_mesh", None)
        body_mesh.set_editor_property("visible", False)
        print("BodyMesh static mesh cleared and hidden.")

    unreal.BlueprintEditorLibrary.compile_blueprint(bp_hound)
    unreal.EditorAssetLibrary.save_loaded_asset(bp_hound)
    print("BP_Hound saved successfully!")

if __name__ == "__main__":
    wire()
