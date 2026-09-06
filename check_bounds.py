import unreal

asset = unreal.load_asset('/Game/Meshes/Test/SM_TestBox')
if asset:
    bounds = asset.get_bounds()
    print(f'>>> TEST BOX BOUNDS: Origin={bounds.origin}, BoxExtent={bounds.box_extent}, SphereRadius={bounds.sphere_radius}')
else:
    print('>>> FAILED TO LOAD ASSET')
