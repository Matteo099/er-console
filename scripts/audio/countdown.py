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

OUTPUT = "countdown" + EXTENSIONS

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

duration = 1.0
samples = []

# Due impulsi: TIC e TAC
events = [
    (0.08, 1450, 0.17),
    (0.56, 1120, 0.20)
]

for n in range(int(SAMPLE_RATE * duration)):
    t = n / SAMPLE_RATE
    sound = 0.0

    for start, frequency, volume in events:
        age = t - start

        if 0 <= age < 0.16:
            envelope = math.exp(-age * 34)
            envelope *= min(1.0, age / 0.002)

            tick = (
                math.sin(2 * math.pi * frequency * age) +
                0.24 * math.sin(2 * math.pi * frequency * 1.73 * age) +
                0.12 * math.sin(2 * math.pi * 230 * age)
            )

            sound += volume * envelope * tick

    samples.append(sound)

save_wav(OUTPUT, samples)
print(f"Creato {OUTPUT}")