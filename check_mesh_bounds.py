import unreal

for name in ['Modular/SM_Wall_Modular_400x300', 'Modular/SM_Floor_Tile_400x400', 'Modular/SM_Ceiling_FluorescentLight', 'Modular/SM_Pillar_Industrial', 'Props/SM_HidingLocker']:
    asset = unreal.load_asset(f'/Game/Meshes/{name}')
    if asset:
        b = asset.get_bounds()
        print(f'>>> {name}: Extent=({b.box_extent.x:.1f}, {b.box_extent.y:.1f}, {b.box_extent.z:.1f}), Origin=({b.origin.x:.1f}, {b.origin.y:.1f}, {b.origin.z:.1f})')
