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

OUTPUT = "button_click" + EXTENSIONS

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

# Durata: 0.20 secondi
duration = 0.20
samples = []

for n in range(int(SAMPLE_RATE * duration)):
    t = n / SAMPLE_RATE

    # Impulso principale
    envelope = math.exp(-t * 38)

    thump = 0.45 * math.sin(2 * math.pi * 115 * t) * envelope

    # Click elettronico
    if t >= 0.012:
        age = t - 0.012
        click_env = math.exp(-age * 95)
        click = 0.22 * (
            math.sin(2 * math.pi * 1850 * age) +
            0.45 * math.sin(2 * math.pi * 2900 * age)
        ) * click_env
    else:
        click = 0

    # Piccola componente metallica
    transient = 0.08 * math.sin(2 * math.pi * 5173 * t) * math.exp(-t * 150)

    fade_in = min(1.0, t * 700)

    samples.append((thump + click + transient) * fade_in)

save_wav(OUTPUT, samples)
print(f"Creato {OUTPUT}")