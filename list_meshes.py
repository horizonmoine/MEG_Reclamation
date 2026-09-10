import unreal

asset_reg = unreal.AssetRegistryHelpers.get_asset_registry()
assets = asset_reg.get_assets_by_class("StaticMesh", True)
print("=== AVAILABLE STATIC MESHES ===")
for a in assets:
    print(a.package_name)
