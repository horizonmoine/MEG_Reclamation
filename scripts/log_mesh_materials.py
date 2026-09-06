import unreal
import os

report_path = r"F:\MEG_Reclamation\scripts\mesh_materials_report.txt"
lines = []

meshes = [
    "/Game/Meshes/Modular/SM_Floor_Tile_400x400",
    "/Game/Meshes/Modular/SM_Wall_Modular_400x300",
    "/Game/Meshes/Modular/SM_Ceiling_Tile_400x400",
    "/Game/Meshes/Modular/SM_Pillar_Industrial",
    "/Game/Meshes/Modular/SM_Ceiling_FluorescentLight",
    "/Game/Meshes/Props/SM_Office_Desk",
    "/Game/Meshes/Props/SM_Office_Chair",
    "/Game/Meshes/Props/SM_Terminal_MEG",
    "/Game/Meshes/Props/SM_HidingLocker",
    "/Game/Meshes/Props/SM_SupplyCrate_MEG",
    "/Game/Meshes/Props/SM_Exit_Sign",
]

for p in meshes:
    m = unreal.load_asset(p)
    if not m:
        lines.append(f"[MISSING] {p}")
        continue
    mats = m.get_editor_property('static_materials')
    lines.append(f"Mesh: {m.get_name()} (sections: {len(mats)})")
    for idx, sm in enumerate(mats):
        mi = sm.get_editor_property('material_interface')
        name = mi.get_name() if mi else "NULL"
        lines.append(f"   Slot {idx} '{sm.get_editor_property('material_slot_name')}': {name}")

with open(report_path, "w", encoding="utf-8") as f:
    f.write("\n".join(lines))
