import wave
import struct
import math
import random
import os

OUTPUT_DIR = r"F:\MEG_Reclamation\RawAudio"
os.makedirs(OUTPUT_DIR, exist_ok=True)
SAMPLE_RATE = 44100

def write_wav(filename, samples):
    filepath = os.path.join(OUTPUT_DIR, filename)
    with wave.open(filepath, 'w') as wav:
        wav.setnchannels(1)  # mono
        wav.setsampwidth(2)  # 16-bit
        wav.setframerate(SAMPLE_RATE)
        max_val = max(max(abs(s) for s in samples), 0.0001)
        int_samples = [int(max(-1.0, min(1.0, s / max_val * 0.95)) * 32767) for s in samples]
        raw_data = struct.pack(f'<{len(int_samples)}h', *int_samples)
        wav.writeframes(raw_data)
    print(f"Generated audio: {filepath}")

# 1. S_Door_Slow_Creak (Discreet slow hinge creak)
samples = []
duration = 1.2
total_samples = int(SAMPLE_RATE * duration)
for i in range(total_samples):
    t = i / SAMPLE_RATE
    env = math.sin(math.pi * (t / duration)) ** 1.5
    freq = 360.0 + 80.0 * math.sin(2.0 * math.pi * 3.5 * t) + 30.0 * math.sin(2.0 * math.pi * 17.0 * t)
    sig = (0.5 * math.sin(2 * math.pi * freq * t) +
           0.3 * math.sin(4 * math.pi * freq * t) +
           0.15 * math.sin(6 * math.pi * freq * t) +
           0.2 * (random.random() * 2.0 - 1.0) * math.exp(-2.0 * t))
    samples.append(sig * env * 0.6)
write_wav("S_Door_Slow_Creak.wav", samples)

# 2. S_Door_Kick_Breach (Explosive kick breach: bass punch + door splinter crash)
samples = []
duration = 0.85
total_samples = int(SAMPLE_RATE * duration)
for i in range(total_samples):
    t = i / SAMPLE_RATE
    sub = math.exp(-12.0 * t) * math.sin(2 * math.pi * 65.0 * math.exp(-6.0 * t) * t)
    crack_env = math.exp(-35.0 * t)
    noise = (random.random() * 2.0 - 1.0) * crack_env * 1.2
    rattle = 0.4 * math.exp(-8.0 * t) * (math.sin(2 * math.pi * 210.0 * t) + math.sin(2 * math.pi * 440.0 * t))
    sig = 0.6 * sub + 0.5 * noise + 0.3 * rattle
    samples.append(sig)
write_wav("S_Door_Kick_Breach.wav", samples)

# 3. S_Spray_AlmondWater (Pressurized aerosol fine mist hiss)
samples = []
duration = 0.9
total_samples = int(SAMPLE_RATE * duration)
for i in range(total_samples):
    t = i / SAMPLE_RATE
    env = min(t / 0.05, 1.0) * math.exp(-1.5 * t)
    hiss = (random.random() * 2.0 - 1.0)
    whistle = 0.25 * math.sin(2 * math.pi * 4800.0 * t) * math.exp(-3.0 * t)
    sig = (0.75 * hiss + whistle) * env
    samples.append(sig)
write_wav("S_Spray_AlmondWater.wav", samples)

# 4. S_Adrenaline_Inject (Mechanical injector snap + pneumatic surge + heart pulse)
samples = []
duration = 0.85
total_samples = int(SAMPLE_RATE * duration)
for i in range(total_samples):
    t = i / SAMPLE_RATE
    snap = math.exp(-90.0 * t) * (math.sin(2 * math.pi * 2800.0 * t) + 0.5 * (random.random() * 2.0 - 1.0))
    surge_env = math.exp(-10.0 * max(0.0, t - 0.03)) if t >= 0.03 else 0.0
    surge = surge_env * (random.random() * 2.0 - 1.0) * 0.6
    heart_t = max(0.0, t - 0.15)
    heart = math.exp(-15.0 * heart_t) * math.sin(2 * math.pi * 55.0 * heart_t) if t >= 0.15 else 0.0
    sig = snap + surge + 0.7 * heart
    samples.append(sig)
write_wav("S_Adrenaline_Inject.wav", samples)

# 5. S_Airlock_Alarm (2-tone oscillating industrial alarm horn)
samples = []
duration = 1.6
total_samples = int(SAMPLE_RATE * duration)
for i in range(total_samples):
    t = i / SAMPLE_RATE
    tone = 580.0 if (int(t / 0.4) % 2 == 0) else 440.0
    tone_sig = (math.sin(2 * math.pi * tone * t) +
                0.35 * math.sin(4 * math.pi * tone * t) +
                0.15 * math.sin(6 * math.pi * tone * t))
    pulse = 0.7 + 0.3 * math.sin(2 * math.pi * 10.0 * t)
    samples.append(tone_sig * pulse * 0.7)
write_wav("S_Airlock_Alarm.wav", samples)

# 6. S_Airlock_Decompress (Massive pneumatic steam purge + deep vibration rumble)
samples = []
duration = 2.4
total_samples = int(SAMPLE_RATE * duration)
for i in range(total_samples):
    t = i / SAMPLE_RATE
    env = min(t / 0.2, 1.0) * math.exp(-0.8 * t)
    rumble = 0.6 * math.sin(2 * math.pi * 38.0 * t) + 0.4 * math.sin(2 * math.pi * 76.0 * t)
    steam = (random.random() * 2.0 - 1.0) * 0.7
    sig = (rumble * 0.5 + steam * 0.5) * env
    samples.append(sig)
write_wav("S_Airlock_Decompress.wav", samples)

# 7. S_Airlock_Chime (High purity two-note electronic clearance chime)
samples = []
duration = 1.3
total_samples = int(SAMPLE_RATE * duration)
for i in range(total_samples):
    t = i / SAMPLE_RATE
    n1 = math.exp(-4.5 * t) * math.sin(2 * math.pi * 1046.5 * t)
    t2 = max(0.0, t - 0.25)
    n2 = math.exp(-4.0 * t2) * math.sin(2 * math.pi * 1318.5 * t2) if t >= 0.25 else 0.0
    sig = 0.5 * n1 + 0.6 * n2
    samples.append(sig)
write_wav("S_Airlock_Chime.wav", samples)

# 8. S_Loot_Heavy_Pickup (Two-handed heavy lift: heavy metallic friction + deep thump)
samples = []
duration = 0.65
total_samples = int(SAMPLE_RATE * duration)
for i in range(total_samples):
    t = i / SAMPLE_RATE
    thump = math.exp(-14.0 * t) * math.sin(2 * math.pi * 80.0 * t)
    friction = math.exp(-8.0 * t) * (random.random() * 2.0 - 1.0) * 0.4
    clink = math.exp(-25.0 * t) * math.sin(2 * math.pi * 1250.0 * t) * 0.3
    sig = 0.6 * thump + friction + clink
    samples.append(sig)
write_wav("S_Loot_Heavy_Pickup.wav", samples)

print("=== ALL GAMEPLAY SFX CREATED SUCCESSFULLY ===")
