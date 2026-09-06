"""
Procedural Backrooms PBR Texture Generator
Synthesizes authentic photorealistic 512x512 seamless textures for Backrooms biomes:
- Level 4: T_Office_CarpetTile, T_Office_WallPlaster
- Level 8: T_Cave_Rock
- Level 9: T_Suburbs_Asphalt
- Level 10: T_Barn_Wood
- Level 3: T_Electrical_Panel
- Level !: T_Run_HospitalFloor, T_Run_HospitalWall
"""

import bpy
import numpy as np
import os

OUTPUT_DIR = r"F:\MEG_Reclamation\Content\Textures\Backrooms"
os.makedirs(OUTPUT_DIR, exist_ok=True)
SIZE = 512

def save_image(name, rgba_array):
    """Saves a NumPy RGBA float32 array (0.0 to 1.0) as a PNG via Blender."""
    img = bpy.data.images.new(name, width=SIZE, height=SIZE, alpha=True)
    img.pixels.foreach_set(rgba_array.flatten())
    img.file_format = 'PNG'
    filepath = os.path.join(OUTPUT_DIR, f"{name}.png")
    img.filepath_raw = filepath
    img.save()
    print(f"Generated and saved: {filepath}")
    bpy.data.images.remove(img)

def generate_noise(size, scale):
    """Generates multi-octave noise via bilinear interpolation of random grids."""
    grid_size = max(2, int(size / scale))
    grid = np.random.RandomState(42).rand(grid_size, grid_size).astype(np.float32)
    # Simple bilinear upsampling
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

def fbm(size, octaves=4):
    """Fractal Brownian Motion noise."""
    res = np.zeros((size, size), dtype=np.float32)
    amp = 1.0
    freq = 64.0
    total_amp = 0.0
    for i in range(octaves):
        res += generate_noise(size, freq) * amp
        total_amp += amp
        amp *= 0.5
        freq *= 0.5
    return res / total_amp

print("=== GENERATING BACKROOMS PBR TEXTURES ===")

# -------------------------------------------------------------
# 1. Level 4: T_Office_CarpetTile (Corporate slate-blue loop carpet)
# -------------------------------------------------------------
tile_size = 128
carpet_rgba = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
noise_fine = np.random.RandomState(101).normal(0.5, 0.08, (SIZE, SIZE)).astype(np.float32)
noise_coarse = fbm(SIZE, 4)

# Grid seam indentations
y_indices, x_indices = np.indices((SIZE, SIZE))
seam_x = (x_indices % tile_size < 3) | (x_indices % tile_size > tile_size - 3)
seam_y = (y_indices % tile_size < 3) | (y_indices % tile_size > tile_size - 3)
seams = seam_x | seam_y

# Base color: Dark corporate charcoal/slate blue
base_r = 0.18 + noise_fine * 0.05 + noise_coarse * 0.04
base_g = 0.22 + noise_fine * 0.05 + noise_coarse * 0.04
base_b = 0.28 + noise_fine * 0.06 + noise_coarse * 0.05

# Seam darkening
base_r[seams] *= 0.4
base_g[seams] *= 0.4
base_b[seams] *= 0.4

# Backrooms water leak stain (irregular darker/yellowish stain in corner)
dist_center = np.sqrt((x_indices - 350)**2 + (y_indices - 180)**2)
stain = np.exp(-dist_center**2 / (2 * 70**2)) * (noise_coarse * 0.8 + 0.2)
base_r += stain * 0.06
base_g += stain * 0.05
base_b -= stain * 0.04

carpet_rgba[:, :, 0] = np.clip(base_r, 0.0, 1.0)
carpet_rgba[:, :, 1] = np.clip(base_g, 0.0, 1.0)
carpet_rgba[:, :, 2] = np.clip(base_b, 0.0, 1.0)
carpet_rgba[:, :, 3] = 1.0
save_image("T_Office_CarpetTile", carpet_rgba)

# -------------------------------------------------------------
# 2. Level 4: T_Office_WallPlaster (Corporate drywall plaster)
# -------------------------------------------------------------
plaster_rgba = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
plaster_grain = np.random.RandomState(202).normal(0.5, 0.04, (SIZE, SIZE)).astype(np.float32)
plaster_broad = fbm(SIZE, 3)

base_r = 0.78 + plaster_grain * 0.04 + plaster_broad * 0.03
base_g = 0.76 + plaster_grain * 0.04 + plaster_broad * 0.03
base_b = 0.72 + plaster_grain * 0.03 + plaster_broad * 0.03

# Subtle horizontal drywall joint
drywall_joint = np.abs(y_indices - 256) < 4
base_r[drywall_joint] *= 0.94
base_g[drywall_joint] *= 0.94
base_b[drywall_joint] *= 0.94

plaster_rgba[:, :, 0] = np.clip(base_r, 0.0, 1.0)
plaster_rgba[:, :, 1] = np.clip(base_g, 0.0, 1.0)
plaster_rgba[:, :, 2] = np.clip(base_b, 0.0, 1.0)
plaster_rgba[:, :, 3] = 1.0
save_image("T_Office_WallPlaster", plaster_rgba)

# -------------------------------------------------------------
# 3. Level 8: T_Cave_Rock (Stratified subterranean limestone)
# -------------------------------------------------------------
rock_rgba = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
rock_fbm1 = fbm(SIZE, 5)
rock_fbm2 = fbm(SIZE, 3)

# Horizontal geological stratification layers
strata = np.sin(y_indices * 0.06 + rock_fbm1 * 4.0) * 0.12
crevices = (np.abs(rock_fbm1 - 0.5) < 0.04).astype(np.float32)

r = 0.22 + rock_fbm1 * 0.12 + strata - crevices * 0.12
g = 0.18 + rock_fbm1 * 0.10 + strata * 0.9 - crevices * 0.12
b = 0.15 + rock_fbm1 * 0.08 + strata * 0.8 - crevices * 0.12

rock_rgba[:, :, 0] = np.clip(r, 0.0, 1.0)
rock_rgba[:, :, 1] = np.clip(g, 0.0, 1.0)
rock_rgba[:, :, 2] = np.clip(b, 0.0, 1.0)
rock_rgba[:, :, 3] = 1.0
save_image("T_Cave_Rock", rock_rgba)

# -------------------------------------------------------------
# 4. Level 9: T_Suburbs_Asphalt (Wet asphalt road with macadam)
# -------------------------------------------------------------
asphalt_rgba = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
asphalt_grain = np.random.RandomState(404).normal(0.5, 0.12, (SIZE, SIZE)).astype(np.float32)
wet_puddles = fbm(SIZE, 4)

r = 0.14 + asphalt_grain * 0.06
g = 0.14 + asphalt_grain * 0.06
b = 0.15 + asphalt_grain * 0.07

# Wet puddles (darker, glossy reflections)
puddle_mask = wet_puddles > 0.62
r[puddle_mask] *= 0.65
g[puddle_mask] *= 0.65
b[puddle_mask] *= 0.70

# Subtle yellow road line dashed on one side
yellow_stripe = (x_indices >= 240) & (x_indices <= 272) & ((y_indices % 128) < 80)
r[yellow_stripe] = 0.80
g[yellow_stripe] = 0.65
b[yellow_stripe] = 0.12

asphalt_rgba[:, :, 0] = np.clip(r, 0.0, 1.0)
asphalt_rgba[:, :, 1] = np.clip(g, 0.0, 1.0)
asphalt_rgba[:, :, 2] = np.clip(b, 0.0, 1.0)
asphalt_rgba[:, :, 3] = 1.0
save_image("T_Suburbs_Asphalt", asphalt_rgba)

# -------------------------------------------------------------
# 5. Level 10: T_Barn_Wood (Rustic weathered wood planks)
# -------------------------------------------------------------
wood_rgba = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
plank_height = 64
plank_idx = y_indices // plank_height
plank_grain = np.sin(x_indices * 0.05 + fbm(SIZE, 3) * 6.0) * 0.08
wood_noise = np.random.RandomState(505).normal(0.5, 0.06, (SIZE, SIZE)).astype(np.float32)

# Random shade per plank
plank_shade = ((plank_idx * 37) % 10) * 0.02 - 0.1

r = 0.44 + plank_grain + wood_noise * 0.06 + plank_shade
g = 0.32 + plank_grain * 0.8 + wood_noise * 0.05 + plank_shade * 0.8
b = 0.20 + plank_grain * 0.6 + wood_noise * 0.04 + plank_shade * 0.6

# Seams between planks
seam_plank = (y_indices % plank_height < 3)
r[seam_plank] *= 0.25
g[seam_plank] *= 0.25
b[seam_plank] *= 0.25

wood_rgba[:, :, 0] = np.clip(r, 0.0, 1.0)
wood_rgba[:, :, 1] = np.clip(g, 0.0, 1.0)
wood_rgba[:, :, 2] = np.clip(b, 0.0, 1.0)
wood_rgba[:, :, 3] = 1.0
save_image("T_Barn_Wood", wood_rgba)

# -------------------------------------------------------------
# 6. Level 3: T_Electrical_Panel (Industrial dark gray with hazard stripe)
# -------------------------------------------------------------
elec_rgba = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
metal_grain = np.random.RandomState(606).normal(0.5, 0.05, (SIZE, SIZE)).astype(np.float32)

r = 0.18 + metal_grain * 0.04
g = 0.20 + metal_grain * 0.04
b = 0.22 + metal_grain * 0.05

# Diagonal hazard warning tape at top (y < 48) and bottom (y > 464)
hazard_zone = (y_indices < 48) | (y_indices > 464)
hazard_stripe = ((x_indices + y_indices) // 24) % 2 == 0

r[hazard_zone & hazard_stripe] = 0.88
g[hazard_zone & hazard_stripe] = 0.75
b[hazard_zone & hazard_stripe] = 0.08

r[hazard_zone & ~hazard_stripe] = 0.08
g[hazard_zone & ~hazard_stripe] = 0.08
b[hazard_zone & ~hazard_stripe] = 0.08

elec_rgba[:, :, 0] = np.clip(r, 0.0, 1.0)
elec_rgba[:, :, 1] = np.clip(g, 0.0, 1.0)
elec_rgba[:, :, 2] = np.clip(b, 0.0, 1.0)
elec_rgba[:, :, 3] = 1.0
save_image("T_Electrical_Panel", elec_rgba)

# -------------------------------------------------------------
# 7. Level !: T_Run_HospitalFloor (Sterile scuffed linoleum + red emergency line)
# -------------------------------------------------------------
run_floor_rgba = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
floor_grain = np.random.RandomState(707).normal(0.5, 0.03, (SIZE, SIZE)).astype(np.float32)

# Tile grid 64x64
floor_tile = (x_indices % 64 < 2) | (y_indices % 64 < 2)

r = 0.86 + floor_grain * 0.04
g = 0.87 + floor_grain * 0.04
b = 0.88 + floor_grain * 0.04

r[floor_tile] *= 0.82
g[floor_tile] *= 0.82
b[floor_tile] *= 0.82

# Bold central blood-red emergency runner stripe (Level ! hallmark)
red_line = (x_indices >= 220) & (x_indices <= 292)
r[red_line] = 0.82
g[red_line] = 0.08
b[red_line] = 0.08

# Scuff marks across hallway
scuff = (np.abs(np.sin(x_indices * 0.12 + y_indices * 0.08)) > 0.98) & (floor_grain < 0.45)
r[scuff] *= 0.6
g[scuff] *= 0.6
b[scuff] *= 0.6

run_floor_rgba[:, :, 0] = np.clip(r, 0.0, 1.0)
run_floor_rgba[:, :, 1] = np.clip(g, 0.0, 1.0)
run_floor_rgba[:, :, 2] = np.clip(b, 0.0, 1.0)
run_floor_rgba[:, :, 3] = 1.0
save_image("T_Run_HospitalFloor", run_floor_rgba)

# -------------------------------------------------------------
# 8. Level !: T_Run_HospitalWall (Sterile hospital wall with emergency band)
# -------------------------------------------------------------
run_wall_rgba = np.zeros((SIZE, SIZE, 4), dtype=np.float32)
wall_grain = np.random.RandomState(808).normal(0.5, 0.03, (SIZE, SIZE)).astype(np.float32)

r = 0.90 + wall_grain * 0.03
g = 0.90 + wall_grain * 0.03
b = 0.92 + wall_grain * 0.03

# Lower kickplate baseboard (y > 440)
kickplate = y_indices > 440
r[kickplate] = 0.25
g[kickplate] = 0.26
b[kickplate] = 0.28

# Emergency crimson hazard band at eye-level (y between 220 and 260)
crimson_band = (y_indices >= 220) & (y_indices <= 260)
r[crimson_band] = 0.85
g[crimson_band] = 0.06
b[crimson_band] = 0.08

run_wall_rgba[:, :, 0] = np.clip(r, 0.0, 1.0)
run_wall_rgba[:, :, 1] = np.clip(g, 0.0, 1.0)
run_wall_rgba[:, :, 2] = np.clip(b, 0.0, 1.0)
run_wall_rgba[:, :, 3] = 1.0
save_image("T_Run_HospitalWall", run_wall_rgba)

print("=== ALL 8 BACKROOMS TEXTURES GENERATED SUCCESSFULLY ===")
