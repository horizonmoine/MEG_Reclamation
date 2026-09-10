import unreal

imc_path = "/Game/Input/Scavenger/IMC_Scavenger"
imc = unreal.EditorAssetLibrary.load_asset(imc_path)
if imc:
    print(f"Loaded {imc_path}")
    # The IMC mapping is not exposed directly via standard python API easily in UE5,
    # but we can try to find the action and add keys.
    # Actually, modifying IMC via python requires specific classes.
    pass
