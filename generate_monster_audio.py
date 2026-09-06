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
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(SAMPLE_RATE)
        max_val = max(max(abs(s) for s in samples), 0.0001)
        int_samples = [int(max(-1.0, min(1.0, s / max_val * 0.95)) * 32767) for s in samples]
        raw_data = struct.pack(f'<{len(int_samples)}h', *int_samples)
        wav.writeframes(raw_data)
    print(f"Generated monster audio: {filepath}")

# 1. S_Hound_Snarl (1.2s guttural snarl)
samples = []
duration = 1.2
total_samples = int(SAMPLE_RATE * duration)
for i in range(total_samples):
    t = i / SAMPLE_RATE
    env = math.sin(math.pi * (t / duration)) ** 0.5
    f_mod = 85.0 + 35.0 * math.sin(2 * math.pi * 7.5 * t)
    sig = (0.5 * math.sin(2 * math.pi * f_mod * t) +
           0.3 * math.sin(2 * math.pi * (f_mod * 2.1) * t) +
           0.35 * (random.random() * 2.0 - 1.0) * math.sin(2 * math.pi * 15.0 * t))
    samples.append(sig * env)
write_wav("S_Hound_Snarl.wav", samples)

# 2. S_Hound_Bite (0.35s snap & crush)
samples = []
duration = 0.35
total_samples = int(SAMPLE_RATE * duration)
for i in range(total_samples):
    t = i / SAMPLE_RATE
    env = math.exp(-22.0 * t)
    sig = (0.7 * math.sin(2 * math.pi * 140 * t) +
           0.5 * (random.random() * 2.0 - 1.0) * math.exp(-30.0 * t) +
           0.4 * math.sin(2 * math.pi * 420 * t))
    samples.append(sig * env)
write_wav("S_Hound_Bite.wav", samples)

# 3. S_Smiler_Distortion (1.5s unsettling static & resonant ring)
samples = []
duration = 1.5
total_samples = int(SAMPLE_RATE * duration)
for i in range(total_samples):
    t = i / SAMPLE_RATE
    env = math.sin(math.pi * (t / duration))
    # Dissonant intervals (tritone 440 & 622 Hz)
    sig = (0.4 * math.sin(2 * math.pi * 440.0 * t) +
           0.4 * math.sin(2 * math.pi * 622.25 * t) +
           0.25 * (random.random() * 2.0 - 1.0) * (0.5 + 0.5 * math.sin(2 * math.pi * 4.0 * t)))
    samples.append(sig * env * 0.6)
write_wav("S_Smiler_Distortion.wav", samples)

# 4. S_Partygoer_Chime (1.6s eerie music box notes)
samples = []
duration = 1.6
total_samples = int(SAMPLE_RATE * duration)
# 4 chromatic chime notes (C6, E6, G#6, B6)
chimes = [1046.5, 1318.5, 1661.2, 1975.5]
for i in range(total_samples):
    t = i / SAMPLE_RATE
    sig = 0.0
    for idx, freq in enumerate(chimes):
        t_note = t - idx * 0.35
        if t_note > 0:
            note_env = math.exp(-6.0 * t_note)
            sig += 0.3 * math.sin(2 * math.pi * freq * t_note) * note_env
    samples.append(sig)
write_wav("S_Partygoer_Chime.wav", samples)

# 5. S_Heartbeat_Panic (0.8s double thump)
samples = []
duration = 0.8
total_samples = int(SAMPLE_RATE * duration)
for i in range(total_samples):
    t = i / SAMPLE_RATE
    sig = 0.0
    # Thump 1 at t=0
    if t < 0.25:
        sig += 0.8 * math.sin(2 * math.pi * 55.0 * t) * math.exp(-14.0 * t)
    # Thump 2 at t=0.25
    t2 = t - 0.22
    if t2 > 0 and t2 < 0.3:
        sig += 0.6 * math.sin(2 * math.pi * 60.0 * t2) * math.exp(-16.0 * t2)
    samples.append(sig)
write_wav("S_Heartbeat_Panic.wav", samples)

print("=== ALL MONSTER SFX GENERATED SUCCESSFULLY ===")
