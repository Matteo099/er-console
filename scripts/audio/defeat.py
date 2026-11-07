import math
import wave
import struct
import os

SAMPLE_RATE = 22050
CHANNELS = 1
BITS = 16
BITRATE = SAMPLE_RATE * CHANNELS * BITS
MAX_BYTES = 4 * 1024 * 1024
EXTENSIONS = '.wav'

OUTPUT = "defeat" + EXTENSIONS

def save_wav(filename, samples):
    peak = max(abs(x) for x in samples) or 1.0
    scale = min(1.0, 0.90 / peak)

    with wave.open(filename, "wb") as wav:
        wav.setnchannels(CHANNELS)
        wav.setsampwidth(BITS // 8)
        wav.setframerate(SAMPLE_RATE)

        data = bytearray()
        for sample in samples:
            value = int(max(-1, min(1, sample * scale)) * 32767)
            data.extend(struct.pack("<h", value))

        wav.writeframes(data)

    if os.path.getsize(filename) > MAX_BYTES:
        os.remove(filename)
        raise ValueError("File audio superiore a 4 MB")

# Sequenza discendente: Sol - Fa - Re - La
notes = [392.00, 349.23, 293.66, 220.00]

duration = 4.0
samples = []

for n in range(int(SAMPLE_RATE * duration)):
    t = n / SAMPLE_RATE
    sound = 0.0

    # Note discendenti
    for i, frequency in enumerate(notes):
        start = 0.12 + i * 0.57
        age = t - start

        if age >= 0:
            envelope = min(1.0, age / 0.04)
            envelope *= math.exp(-age * (0.72 if i == 3 else 1.8))

            tone = (
                math.sin(2 * math.pi * frequency * age) +
                0.32 * math.sin(2 * math.pi * frequency * 2 * age)
            )

            sound += 0.17 * envelope * tone

    # Drone grave con frequenza discendente
    drone_env = min(1.0, t / 0.35) * math.exp(-t * 0.55)

    falling_frequency = 92 - 17 * min(1.0, t / 2.8)

    drone = (
        math.sin(2 * math.pi * falling_frequency * t) +
        0.22 * math.sin(2 * math.pi * falling_frequency * 1.5 * t)
    )

    sound += 0.19 * drone_env * drone

    # Dissolvenza
    fade = min(1.0, t / 0.04, (duration - t) / 0.65)

    samples.append(sound * max(0.0, fade))

save_wav(OUTPUT, samples)
print(f"Creato {OUTPUT}")