import wave
import struct
import math
import random
import os

OUTPUT_DIR = r"F:\MEG_Reclamation\RawAssets\Audio"
os.makedirs(OUTPUT_DIR, exist_ok=True)
SAMPLE_RATE = 44100

def write_wav(filename, samples):
    filepath = os.path.join(OUTPUT_DIR, filename)
    with wave.open(filepath, 'w') as wav:
        wav.setnchannels(1) # mono
        wav.setsampwidth(2) # 16-bit
        wav.setframerate(SAMPLE_RATE)
        # normalize and convert to 16-bit int
        max_val = max(max(abs(s) for s in samples), 0.0001)
        int_samples = [int(max(-1.0, min(1.0, s / max_val * 0.95)) * 32767) for s in samples]
        raw_data = struct.pack(f'<{len(int_samples)}h', *int_samples)
        wav.writeframes(raw_data)
    print(f"Generated audio: {filepath}")

# 1. S_Loot_Impact_Metal
samples = []
duration = 0.4
total_samples = int(SAMPLE_RATE * duration)
for i in range(total_samples):
    t = i / SAMPLE_RATE
    env = math.exp(-18.0 * t)
    # Metallic harmonics
    sig = (0.5 * math.sin(2 * math.pi * 1420 * t) +
           0.3 * math.sin(2 * math.pi * 2850 * t) +
           0.2 * math.sin(2 * math.pi * 4600 * t) +
           0.15 * math.sin(2 * math.pi * 830 * t) +
           0.25 * (random.random() * 2.0 - 1.0) * math.exp(-40.0 * t))
    samples.append(sig * env)
write_wav("S_Loot_Impact_Metal.wav", samples)

# 2. S_Loot_Impact_Heavy
samples = []
duration = 0.5
total_samples = int(SAMPLE_RATE * duration)
for i in range(total_samples):
    t = i / SAMPLE_RATE
    env = math.exp(-12.0 * t)
    # Low thud with body
    sig = (0.7 * math.sin(2 * math.pi * (110.0 * math.exp(-10.0 * t)) * t) +
           0.3 * math.sin(2 * math.pi * 65.0 * t) +
           0.4 * (random.random() * 2.0 - 1.0) * math.exp(-35.0 * t))
    samples.append(sig * env)
write_wav("S_Loot_Impact_Heavy.wav", samples)

# 3. S_Loot_Impact_Plastic
samples = []
duration = 0.25
total_samples = int(SAMPLE_RATE * duration)
for i in range(total_samples):
    t = i / SAMPLE_RATE
    env = math.exp(-25.0 * t)
    sig = (0.6 * math.sin(2 * math.pi * 950 * t) +
           0.3 * math.sin(2 * math.pi * 1800 * t) +
           0.3 * (random.random() * 2.0 - 1.0) * math.exp(-50.0 * t))
    samples.append(sig * env)
write_wav("S_Loot_Impact_Plastic.wav", samples)

# 4. S_Footstep_Carpet_01 & 02
for step_idx in [1, 2]:
    samples = []
    duration = 0.22
    total_samples = int(SAMPLE_RATE * duration)
    pitch_mod = 1.0 if step_idx == 1 else 1.15
    for i in range(total_samples):
        t = i / SAMPLE_RATE
        env = math.exp(-20.0 * t)
        # Muffled low pass noise & slight heel thud
        sig = (0.6 * math.sin(2 * math.pi * (80.0 * pitch_mod) * t) +
               0.3 * (random.random() * 2.0 - 1.0) * math.exp(-15.0 * t))
        samples.append(sig * env)
    write_wav(f"S_Footstep_Carpet_0{step_idx}.wav", samples)

# 5. S_Footstep_Concrete_01 & 02
for step_idx in [1, 2]:
    samples = []
    duration = 0.2
    total_samples = int(SAMPLE_RATE * duration)
    freq = 220.0 if step_idx == 1 else 250.0
    for i in range(total_samples):
        t = i / SAMPLE_RATE
        env = math.exp(-24.0 * t)
        sig = (0.5 * math.sin(2 * math.pi * freq * t) +
               0.4 * (random.random() * 2.0 - 1.0) * math.exp(-20.0 * t))
        samples.append(sig * env)
    write_wav(f"S_Footstep_Concrete_0{step_idx}.wav", samples)

# 6. S_Fluorescent_Hum (Seamless 2.0s loop)
samples = []
duration = 2.0
total_samples = int(SAMPLE_RATE * duration)
for i in range(total_samples):
    t = i / SAMPLE_RATE
    # 60Hz power hum with odd and even harmonics (120, 180, 240, 360, 480 Hz)
    sig = (0.45 * math.sin(2 * math.pi * 60 * t) +
           0.30 * math.sin(2 * math.pi * 120 * t) +
           0.18 * math.sin(2 * math.pi * 180 * t) +
           0.12 * math.sin(2 * math.pi * 240 * t) +
           0.08 * math.sin(2 * math.pi * 360 * t) +
           0.04 * math.sin(2 * math.pi * 480 * t) +
           0.03 * (random.random() * 2.0 - 1.0))
    samples.append(sig * 0.4)
write_wav("S_Fluorescent_Hum.wav", samples)

# 7. S_Terminal_Beep
samples = []
duration = 0.12
total_samples = int(SAMPLE_RATE * duration)
for i in range(total_samples):
    t = i / SAMPLE_RATE
    env = 1.0 - (t / duration)
    sig = math.sin(2 * math.pi * 880.0 * t) # A5 tone
    samples.append(sig * env)
write_wav("S_Terminal_Beep.wav", samples)

# 8. S_Geiger_Click
samples = []
duration = 0.05
total_samples = int(SAMPLE_RATE * duration)
for i in range(total_samples):
    t = i / SAMPLE_RATE
    env = math.exp(-120.0 * t)
    sig = (random.random() * 2.0 - 1.0) * 0.8 + 0.5 * math.sin(2 * math.pi * 3500 * t)
    samples.append(sig * env)
write_wav("S_Geiger_Click.wav", samples)

print("=== ALL SFX GENERATED SUCCESSFULLY ===")
