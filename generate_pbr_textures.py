"""
================================================================================
M.E.G. RECLAMATION — Photorealistic PBR Backrooms Texture Synthesizer
================================================================================
Generates 1024x1024 seamless PBR texture sets (Albedo, Normal, ORM) for all Backrooms biomes:
- BaseColor (.png)
- Normal Map (_N.png) with tangent-space vector normals (inverting G for Unreal Engine standard)
- ORM Map (_ORM.png) where R = Ambient Occlusion, G = Roughness, B = Metallic
"""

import bpy
import numpy as np
import os

OUTPUT_DIR = r"F:\MEG_Reclamation\Content\Textures\Backrooms"
os.makedirs(OUTPUT_DIR, exist_ok=True)
SIZE = 1024

def save_image(name, rgba_array):
    """Saves a NumPy RGBA float32 array (0.0 to 1.0) as a 1024x1024 PNG via Blender."""
    rgba_clamped = np.clip(rgba_array, 0.0, 1.0).astype(np.float32)
    img = bpy.data.images.new(name, width=SIZE, height=SIZE, alpha=True)
    img.pixels.foreach_set(rgba_clamped.flatten())
    img.file_format = 'PNG'
    filepath = os.path.join(OUTPUT_DIR, f"{name}.png")
    img.filepath_raw = filepath
    img.save()
    print(f"[PBR] Saved: {filepath}")
    bpy.data.images.remove(img)

def compute_normal_map_from_height(height_map, strength=2.5):
    """Computes a tangent-space normal map from a 2D height map with UE inverted G."""
    gy, gx = np.gradient(height_map.astype(np.float32))
    nx = -gx * strength
    ny = gy * strength  # UE standard: inverted green
    nz = np.ones_like(height_map, dtype=np.float32)
    
    length = np.sqrt(nx**2 + ny**2 + nz**2)
    length = np.maximum(length, 1e-6)
    
    nx /= length
    ny /= length
    nz /= length
    
    normal_rgba = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
    normal_rgba[:, :, 0] = nx * 0.5 + 0.5
    normal_rgba[:, :, 1] = ny * 0.5 + 0.5
    normal_rgba[:, :, 2] = nz * 0.5 + 0.5
    normal_rgba[:, :, 3] = 1.0
    return normal_rgba

def generate_noise(size, scale, seed=42):
    """Bilinear interpolated noise."""
    grid_size = max(2, int(size / scale))
    grid = np.random.RandomState(seed).rand(grid_size, grid_size).astype(np.float32)
    x = np.linspace(0, grid_size - 1, size)
    y = np.linspace(0, grid_size - 1, size)
    x0 = np.floor(x).astype(int)
    x1 = np.clip(x0 + 1, 0, grid_size - 1)
    y0 = np.floor(y).astype(int)
    y1 = np.clip(y0 + 1, 0, grid_size - 1)
    
    wx = (x - x0).reshape(1, size)
    wy = (y - y0).reshape(size, 1)
    
    top = (1 - wx) * grid[y0[:, None], x0] + wx * grid[y0[:, None], x1]
    bottom = (1 - wx) * grid[y1[:, None], x0] + wx * grid[y1[:, None], x1]
    return ((1 - wy) * top + wy * bottom).astype(np.float32)

def fbm(size, octaves=5, seed=42):
    """Multi-octave fractal brownian motion."""
    res = np.zeros((size, size), dtype=np.float32)
    amp = 1.0
    freq = 64.0
    total_amp = 0.0
    for i in range(octaves):
        res += generate_noise(size, freq, seed=seed + i * 17) * amp
        total_amp += amp
        amp *= 0.5
        freq *= 0.5
    return res / total_amp

print("==================================================")
print("  GENERATING COMPREHENSIVE BACKROOMS PBR TEXTURES")
print("==================================================")

y_indices, x_indices = np.indices((SIZE, SIZE))

# 1. Level 0: T_Lobby_Wallpaper
print("Generating: T_Lobby_Wallpaper (Albedo, Normal, ORM)...")
stripe_freq = 32
stripe_profile = np.sin(x_indices * (2.0 * np.pi / stripe_freq)) * 0.5 + 0.5
fine_paper = generate_noise(SIZE, 4.0, seed=101) * 0.08
broad_grunge = fbm(SIZE, 4, seed=102)
seam_mask_f = ((x_indices % 256 < 2) | (x_indices % 256 > 254)).astype(np.float32)
bottom_stain = np.clip((y_indices - 800) / 224.0, 0.0, 1.0) * (broad_grunge * 0.5 + 0.5)

base_r = (0.86 + stripe_profile * 0.05 + fine_paper - bottom_stain * 0.22) * (1.0 - seam_mask_f * 0.15)
base_g = (0.76 + stripe_profile * 0.04 + fine_paper - bottom_stain * 0.28) * (1.0 - seam_mask_f * 0.15)
base_b = (0.36 + stripe_profile * 0.03 + fine_paper - bottom_stain * 0.15) * (1.0 - seam_mask_f * 0.15)

save_image("T_Lobby_Wallpaper", np.stack([base_r, base_g, base_b, np.ones((SIZE, SIZE))], axis=-1))
height_wallpaper = stripe_profile * 0.25 + fine_paper * 0.4 - seam_mask_f * 0.15
save_image("T_Lobby_Wallpaper_N", compute_normal_map_from_height(height_wallpaper, strength=2.2))

orm_wallpaper = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
orm_wallpaper[:, :, 0] = np.clip(1.0 - (seam_mask_f * 0.4 + bottom_stain * 0.3), 0.0, 1.0)
orm_wallpaper[:, :, 1] = np.clip(0.85 - bottom_stain * 0.40 + fine_paper * 0.1, 0.1, 1.0)
orm_wallpaper[:, :, 2] = 0.0
orm_wallpaper[:, :, 3] = 1.0
save_image("T_Lobby_Wallpaper_ORM", orm_wallpaper)

# 2. Level 0: T_Lobby_Carpet
print("Generating: T_Lobby_Carpet (Albedo, Normal, ORM)...")
carpet_fine = generate_noise(SIZE, 3.0, seed=201)
carpet_loops = (np.sin(x_indices * 0.8) * np.cos(y_indices * 0.8)) * 0.5 + 0.5
carpet_stains = fbm(SIZE, 5, seed=202)

dist_puddle1 = np.sqrt((x_indices - 320)**2 + (y_indices - 400)**2)
dist_puddle2 = np.sqrt((x_indices - 780)**2 + (y_indices - 820)**2)
wet_mask = np.exp(-dist_puddle1**2 / (2 * 140**2)) + np.exp(-dist_puddle2**2 / (2 * 180**2))
wet_mask = np.clip(wet_mask * (carpet_stains * 0.8 + 0.4), 0.0, 1.0)

c_r = 0.44 + carpet_fine * 0.06 - wet_mask * 0.22
c_g = 0.36 + carpet_fine * 0.05 - wet_mask * 0.20
c_b = 0.18 + carpet_fine * 0.03 - wet_mask * 0.10

save_image("T_Lobby_Carpet", np.stack([c_r, c_g, c_b, np.ones((SIZE, SIZE))], axis=-1))
height_carpet = carpet_loops * 0.3 + carpet_fine * 0.35 - wet_mask * 0.15
save_image("T_Lobby_Carpet_N", compute_normal_map_from_height(height_carpet, strength=3.0))

orm_carpet = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
orm_carpet[:, :, 0] = np.clip(1.0 - carpet_loops * 0.25 - wet_mask * 0.2, 0.0, 1.0)
orm_carpet[:, :, 1] = np.clip(0.92 - wet_mask * 0.65, 0.05, 1.0)
orm_carpet[:, :, 2] = 0.0
orm_carpet[:, :, 3] = 1.0
save_image("T_Lobby_Carpet_ORM", orm_carpet)

# 3. Level 0: T_Lobby_Ceiling
print("Generating: T_Lobby_Ceiling (Albedo, Normal, ORM)...")
tile_grid_f = ((x_indices % 512 < 4) | (x_indices % 512 > 508) | (y_indices % 512 < 4) | (y_indices % 512 > 508)).astype(np.float32)
acoustic_holes = (generate_noise(SIZE, 6.0, seed=301) < 0.18).astype(np.float32)
water_ring = np.exp(-((np.sqrt((x_indices - 480)**2 + (y_indices - 520)**2) - 180)**2) / (2 * 25**2)) * 0.35

ceil_r = 0.88 - tile_grid_f * 0.35 - acoustic_holes * 0.25 + water_ring * 0.08
ceil_g = 0.86 - tile_grid_f * 0.35 - acoustic_holes * 0.25 + water_ring * 0.06
ceil_b = 0.80 - tile_grid_f * 0.35 - acoustic_holes * 0.25 - water_ring * 0.05

save_image("T_Lobby_Ceiling", np.stack([ceil_r, ceil_g, ceil_b, np.ones((SIZE, SIZE))], axis=-1))
height_ceil = -tile_grid_f * 0.4 - acoustic_holes * 0.3 + generate_noise(SIZE, 5.0, seed=302) * 0.1
save_image("T_Lobby_Ceiling_N", compute_normal_map_from_height(height_ceil, strength=2.5))

orm_ceil = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
orm_ceil[:, :, 0] = np.clip(1.0 - tile_grid_f * 0.5 - acoustic_holes * 0.4, 0.0, 1.0)
orm_ceil[:, :, 1] = 0.88
orm_ceil[:, :, 2] = tile_grid_f * 0.4
orm_ceil[:, :, 3] = 1.0
save_image("T_Lobby_Ceiling_ORM", orm_ceil)

# 4. Level 1: T_Concrete_Industrial
print("Generating: T_Concrete_Industrial (Albedo, Normal, ORM)...")
conc_noise = fbm(SIZE, 6, seed=401)
conc_fine = generate_noise(SIZE, 2.5, seed=402)
expansion_joints_f = (((x_indices % 512 < 3) | (x_indices % 512 > 509)) | ((y_indices % 512 < 3) | (y_indices % 512 > 509))).astype(np.float32)
oil_stain = np.exp(-((x_indices - 600)**2 + (y_indices - 350)**2) / (2 * 110**2)) * 0.45

conc_r = 0.42 + conc_noise * 0.1 + conc_fine * 0.05 - expansion_joints_f * 0.25 - oil_stain * 0.3
conc_g = 0.44 + conc_noise * 0.1 + conc_fine * 0.05 - expansion_joints_f * 0.25 - oil_stain * 0.3
conc_b = 0.46 + conc_noise * 0.1 + conc_fine * 0.05 - expansion_joints_f * 0.25 - oil_stain * 0.25

save_image("T_Concrete_Industrial", np.stack([conc_r, conc_g, conc_b, np.ones((SIZE, SIZE))], axis=-1))
height_conc = conc_noise * 0.3 + conc_fine * 0.2 - expansion_joints_f * 0.45
save_image("T_Concrete_Industrial_N", compute_normal_map_from_height(height_conc, strength=2.8))

orm_conc = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
orm_conc[:, :, 0] = np.clip(1.0 - expansion_joints_f * 0.5, 0.0, 1.0)
orm_conc[:, :, 1] = np.clip(0.80 - oil_stain * 0.60, 0.1, 1.0)
orm_conc[:, :, 2] = 0.0
orm_conc[:, :, 3] = 1.0
save_image("T_Concrete_Industrial_ORM", orm_conc)

# 5. Level 2: T_Corrugated_Rust
print("Generating: T_Corrugated_Rust (Albedo, Normal, ORM)...")
corrugation = np.sin(x_indices * (2.0 * np.pi / 64.0)) * 0.5 + 0.5
rust_fbm = fbm(SIZE, 5, seed=501)
rust_mask_f = (rust_fbm > 0.42).astype(np.float32)

rust_r = rust_mask_f * (0.55 + rust_fbm * 0.2) + (1.0 - rust_mask_f) * (0.28 + corrugation * 0.08)
rust_g = rust_mask_f * (0.22 + rust_fbm * 0.1) + (1.0 - rust_mask_f) * (0.29 + corrugation * 0.08)
rust_b = rust_mask_f * (0.08 + rust_fbm * 0.05) + (1.0 - rust_mask_f) * (0.31 + corrugation * 0.08)

save_image("T_Corrugated_Rust", np.stack([rust_r, rust_g, rust_b, np.ones((SIZE, SIZE))], axis=-1))
height_rust = corrugation * 0.75 + (rust_mask_f * rust_fbm) * 0.25
save_image("T_Corrugated_Rust_N", compute_normal_map_from_height(height_rust, strength=3.5))

orm_rust = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
orm_rust[:, :, 0] = np.clip(0.6 + corrugation * 0.4, 0.0, 1.0)
orm_rust[:, :, 1] = rust_mask_f * 0.88 + (1.0 - rust_mask_f) * 0.35
orm_rust[:, :, 2] = rust_mask_f * 0.05 + (1.0 - rust_mask_f) * 0.85
orm_rust[:, :, 3] = 1.0
save_image("T_Corrugated_Rust_ORM", orm_rust)

# 6. Level 3: T_Electrical_Panel
print("Generating: T_Electrical_Panel (Albedo, Normal, ORM)...")
panel_border_f = ((x_indices < 16) | (x_indices > 1008) | (y_indices < 16) | (y_indices > 1008)).astype(np.float32)
hazard_diag = (((x_indices + y_indices) // 32) % 2 == 0).astype(np.float32)
hazard_zone = ((y_indices > 40) & (y_indices < 140)).astype(np.float32)

p_r = np.full((SIZE, SIZE), 0.24, dtype=np.float32)
p_g = np.full((SIZE, SIZE), 0.26, dtype=np.float32)
p_b = np.full((SIZE, SIZE), 0.28, dtype=np.float32)

p_r = p_r * (1.0 - hazard_zone) + hazard_zone * (hazard_diag * 0.90 + (1.0 - hazard_diag) * 0.10)
p_g = p_g * (1.0 - hazard_zone) + hazard_zone * (hazard_diag * 0.75 + (1.0 - hazard_diag) * 0.10)
p_b = p_b * (1.0 - hazard_zone) + hazard_zone * (hazard_diag * 0.10 + (1.0 - hazard_diag) * 0.10)

corner_screws_f = (((np.abs(x_indices - 32) < 8) & (np.abs(y_indices - 32) < 8)) | \
                   ((np.abs(x_indices - 992) < 8) & (np.abs(y_indices - 32) < 8)) | \
                   ((np.abs(x_indices - 32) < 8) & (np.abs(y_indices - 992) < 8)) | \
                   ((np.abs(x_indices - 992) < 8) & (np.abs(y_indices - 992) < 8))).astype(np.float32)
p_r = p_r * (1.0 - corner_screws_f) + corner_screws_f * 0.7
p_g = p_g * (1.0 - corner_screws_f) + corner_screws_f * 0.7
p_b = p_b * (1.0 - corner_screws_f) + corner_screws_f * 0.7

save_image("T_Electrical_Panel", np.stack([p_r, p_g, p_b, np.ones((SIZE, SIZE))], axis=-1))
height_panel = -panel_border_f * 0.4 + corner_screws_f * 0.35 + generate_noise(SIZE, 5.0, seed=601) * 0.05
save_image("T_Electrical_Panel_N", compute_normal_map_from_height(height_panel, strength=2.5))

orm_panel = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
orm_panel[:, :, 0] = np.clip(1.0 - panel_border_f * 0.5, 0.0, 1.0)
orm_panel[:, :, 1] = 0.42
orm_panel[:, :, 2] = 0.70
orm_panel[:, :, 3] = 1.0
save_image("T_Electrical_Panel_ORM", orm_panel)

# 7. Level 4: T_Office_CarpetTile & T_Office_WallPlaster
print("Generating: T_Office_CarpetTile & T_Office_WallPlaster (Albedo, Normal, ORM)...")
tile_mod_f = (((x_indices % 256 < 3) | (x_indices % 256 > 253)) | ((y_indices % 256 < 3) | (y_indices % 256 > 253))).astype(np.float32)
carpet_tex = fbm(SIZE, 5, seed=701)

off_r = 0.18 + carpet_tex * 0.04 - tile_mod_f * 0.1
off_g = 0.22 + carpet_tex * 0.04 - tile_mod_f * 0.1
off_b = 0.28 + carpet_tex * 0.05 - tile_mod_f * 0.12

save_image("T_Office_CarpetTile", np.stack([off_r, off_g, off_b, np.ones((SIZE, SIZE))], axis=-1))
save_image("T_Office_CarpetTile_N", compute_normal_map_from_height(-tile_mod_f * 0.4 + carpet_tex * 0.2, strength=2.6))
save_image("T_Office_CarpetTile_ORM", np.stack([np.clip(1.0 - tile_mod_f * 0.4, 0.0, 1.0), np.full((SIZE, SIZE), 0.88), np.zeros((SIZE, SIZE)), np.ones((SIZE, SIZE))], axis=-1))

plaster_grain = generate_noise(SIZE, 3.0, seed=702) * 0.12
save_image("T_Office_WallPlaster", np.stack([0.84 + plaster_grain, 0.83 + plaster_grain, 0.80 + plaster_grain, np.ones((SIZE, SIZE))], axis=-1))
save_image("T_Office_WallPlaster_N", compute_normal_map_from_height(plaster_grain, strength=1.8))
save_image("T_Office_WallPlaster_ORM", np.stack([np.ones((SIZE, SIZE)), np.full((SIZE, SIZE), 0.85), np.zeros((SIZE, SIZE)), np.ones((SIZE, SIZE))], axis=-1))

# 8. Level 8: T_Cave_Rock
print("Generating: T_Cave_Rock (Albedo, Normal, ORM)...")
rock_fbm = fbm(SIZE, 6, seed=801)
rock_strata = np.sin((y_indices + rock_fbm * 80.0) * 0.05) * 0.5 + 0.5
rock_wet = np.clip(rock_fbm - 0.45, 0.0, 1.0) * 2.0
save_image("T_Cave_Rock", np.stack([0.28 + rock_fbm * 0.12 - rock_wet * 0.12, 0.24 + rock_fbm * 0.10 - rock_wet * 0.10, 0.20 + rock_fbm * 0.08 - rock_wet * 0.08, np.ones((SIZE, SIZE))], axis=-1))
save_image("T_Cave_Rock_N", compute_normal_map_from_height(rock_fbm * 0.6 + rock_strata * 0.3, strength=3.8))
save_image("T_Cave_Rock_ORM", np.stack([np.clip(1.0 - rock_strata * 0.3, 0.0, 1.0), np.clip(0.92 - rock_wet * 0.70, 0.1, 1.0), np.zeros((SIZE, SIZE)), np.ones((SIZE, SIZE))], axis=-1))

# 9. Level 9: T_Suburbs_Asphalt
print("Generating: T_Suburbs_Asphalt (Albedo, Normal, ORM)...")
asphalt_gravel = generate_noise(SIZE, 2.0, seed=901)
asphalt_cracks_f = (((generate_noise(SIZE, 128.0, seed=902) > 0.48) & (generate_noise(SIZE, 128.0, seed=902) < 0.52))).astype(np.float32)
save_image("T_Suburbs_Asphalt", np.stack([0.14 + asphalt_gravel * 0.08 - asphalt_cracks_f * 0.08, 0.14 + asphalt_gravel * 0.08 - asphalt_cracks_f * 0.08, 0.15 + asphalt_gravel * 0.08 - asphalt_cracks_f * 0.08, np.ones((SIZE, SIZE))], axis=-1))
save_image("T_Suburbs_Asphalt_N", compute_normal_map_from_height(asphalt_gravel * 0.35 - asphalt_cracks_f * 0.4, strength=3.0))
save_image("T_Suburbs_Asphalt_ORM", np.stack([np.ones((SIZE, SIZE)), np.full((SIZE, SIZE), 0.72), np.zeros((SIZE, SIZE)), np.ones((SIZE, SIZE))], axis=-1))

# 10. Level 10: T_Barn_Wood
print("Generating: T_Barn_Wood (Albedo, Normal, ORM)...")
plank_seam_f = (((x_indices % 128 < 3) | (x_indices % 128 > 125))).astype(np.float32)
wood_grain = np.sin((y_indices + generate_noise(SIZE, 64.0, seed=1001) * 40.0) * 0.2) * 0.5 + 0.5
save_image("T_Barn_Wood", np.stack([0.52 + wood_grain * 0.1 - plank_seam_f * 0.3, 0.38 + wood_grain * 0.08 - plank_seam_f * 0.3, 0.24 + wood_grain * 0.06 - plank_seam_f * 0.25, np.ones((SIZE, SIZE))], axis=-1))
save_image("T_Barn_Wood_N", compute_normal_map_from_height(wood_grain * 0.3 - plank_seam_f * 0.5, strength=2.8))
save_image("T_Barn_Wood_ORM", np.stack([np.clip(1.0 - plank_seam_f * 0.6, 0.0, 1.0), np.full((SIZE, SIZE), 0.82), np.zeros((SIZE, SIZE)), np.ones((SIZE, SIZE))], axis=-1))

# 11. Level 37: T_Poolrooms_Tile
print("Generating: T_Poolrooms_Tile (Albedo, Normal, ORM)...")
tile_cell = 64
grout_f = (((x_indices % tile_cell < 3) | (x_indices % tile_cell > tile_cell - 3)) | \
           ((y_indices % tile_cell < 3) | (y_indices % tile_cell > tile_cell - 3))).astype(np.float32)
tile_edge_x = np.minimum(x_indices % tile_cell, tile_cell - (x_indices % tile_cell))
tile_edge_y = np.minimum(y_indices % tile_cell, tile_cell - (y_indices % tile_cell))
bevel = np.clip(np.minimum(tile_edge_x, tile_edge_y).astype(np.float32) / 6.0, 0.0, 1.0)
save_image("T_Poolrooms_Tile", np.stack([grout_f * 0.20 + (1.0 - grout_f) * (0.94 + bevel * 0.04),
                                         grout_f * 0.45 + (1.0 - grout_f) * (0.98 + bevel * 0.02),
                                         grout_f * 0.55 + (1.0 - grout_f) * 1.00,
                                         np.ones((SIZE, SIZE))], axis=-1))
save_image("T_Poolrooms_Tile_N", compute_normal_map_from_height(bevel * 0.45 - grout_f * 0.35, strength=3.2))
save_image("T_Poolrooms_Tile_ORM", np.stack([grout_f * 0.25 + (1.0 - grout_f) * 1.0,
                                             grout_f * 0.65 + (1.0 - grout_f) * 0.05,
                                             np.zeros((SIZE, SIZE)),
                                             np.ones((SIZE, SIZE))], axis=-1))

# 12. Level !: T_Run_HospitalFloor & T_Run_HospitalWall
print("Generating: T_Run_HospitalFloor & T_Run_HospitalWall (Albedo, Normal, ORM)...")
hosp_grid_f = (((x_indices % 256 < 2) | (x_indices % 256 > 254)) | ((y_indices % 256 < 2) | (y_indices % 256 > 254))).astype(np.float32)
save_image("T_Run_HospitalFloor", np.stack([0.88 - hosp_grid_f * 0.15, 0.88 - hosp_grid_f * 0.15, 0.90 - hosp_grid_f * 0.15, np.ones((SIZE, SIZE))], axis=-1))
save_image("T_Run_HospitalFloor_N", compute_normal_map_from_height(-hosp_grid_f * 0.3, strength=1.5))
save_image("T_Run_HospitalFloor_ORM", np.stack([np.ones((SIZE, SIZE)), np.full((SIZE, SIZE), 0.18), np.zeros((SIZE, SIZE)), np.ones((SIZE, SIZE))], axis=-1))

red_stripe_f = ((y_indices > 460) & (y_indices < 540)).astype(np.float32)
hw_r = red_stripe_f * 0.85 + (1.0 - red_stripe_f) * 0.92
hw_g = red_stripe_f * 0.08 + (1.0 - red_stripe_f) * 0.92
hw_b = red_stripe_f * 0.08 + (1.0 - red_stripe_f) * 0.92

save_image("T_Run_HospitalWall", np.stack([hw_r, hw_g, hw_b, np.ones((SIZE, SIZE))], axis=-1))
save_image("T_Run_HospitalWall_N", compute_normal_map_from_height(np.zeros((SIZE, SIZE)), strength=1.0))
save_image("T_Run_HospitalWall_ORM", np.stack([np.ones((SIZE, SIZE)), np.full((SIZE, SIZE), 0.30), np.zeros((SIZE, SIZE)), np.ones((SIZE, SIZE))], axis=-1))

print("==================================================")
print("  ALL 36 PBR TEXTURE ASSETS GENERATED!")
print("==================================================")
