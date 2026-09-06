import os
import math
import wave
import struct
import random

sample_rate = 44100
out_dir = 'F:/MEG_Reclamation/RawAudio'
os.makedirs(out_dir, exist_ok=True)

def write_wav(filename, data):
    with wave.open(filename, 'w') as wav_file:
        wav_file.setnchannels(1)
        wav_file.setsampwidth(2)
        wav_file.setframerate(sample_rate)
        for sample in data:
            wav_file.writeframes(struct.pack('<h', int(sample)))

def generate_noise(duration, volume=0.5):
    samples = int(sample_rate * duration)
    return [random.gauss(0, 1) * 32767 * volume for _ in range(samples)]

def generate_sine(duration, freq, volume=0.5):
    samples = int(sample_rate * duration)
    return [math.sin(2 * math.pi * freq * (i / sample_rate)) * 32767 * volume for i in range(samples)]

entity_sounds = [
    'S_Clump_Gurgle', 'S_Clump_Drag', 'S_Deathmoth_Flutter', 'S_Deathmoth_Screech',
    'S_Duller_Growl', 'S_Duller_Rush', 'S_Jerry_Whisper', 'S_Jerry_Laugh',
    'S_Skinwalker_Mimic', 'S_Skinwalker_Scream', 'S_Watcher_Hum', 'S_Watcher_Alert',
    'S_Wretch_Snarl', 'S_Wretch_Lunge'
]

for s in entity_sounds:
    data = generate_noise(0.5, 0.3)
    # clip
    data = [max(min(x, 32767), -32768) for x in data]
    write_wav(os.path.join(out_dir, f"{s}.wav"), data)

ambient_sounds = [
    'S_Ambient_Lobby', 'S_Ambient_HabitableZone', 'S_Ambient_PipeDreams',
    'S_Ambient_Electrical', 'S_Ambient_Office', 'S_Ambient_Cave',
    'S_Ambient_Suburbs', 'S_Ambient_WheatFields', 'S_Ambient_Poolrooms', 'S_Ambient_Run'
]

for s in ambient_sounds:
    data = generate_sine(1.0, 150, 0.2)
    write_wav(os.path.join(out_dir, f"{s}.wav"), data)

print("Audio generation complete.")
