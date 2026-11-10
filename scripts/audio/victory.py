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

OUTPUT = "victory" + EXTENSIONS

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

# Note della sequenza ascendente: Do - Mi - Sol - Do - Mi
notes = [523.25, 659.25, 783.99, 1046.50, 1318.51]

duration = 3.6
samples = []

for n in range(int(SAMPLE_RATE * duration)):
    t = n / SAMPLE_RATE
    sound = 0.0

    # Sequenza di campane elettroniche
    for i, frequency in enumerate(notes):
        start = 0.10 + i * 0.34
        age = t - start

        if age >= 0:
            attack = min(1.0, age / 0.035)
            decay = math.exp(-age * (1.15 if i < 4 else 0.55))
            envelope = attack * decay

            tone = (
                math.sin(2 * math.pi * frequency * age) +
                0.28 * math.sin(2 * math.pi * frequency * 2.01 * age) +
                0.10 * math.sin(2 * math.pi * frequency * 3.98 * age)
            )

            sound += 0.14 * envelope * tone / (1 + 0.12 * i)

    # Accordo finale
    bed_age = t - 1.55

    if bed_age > 0:
        bed_env = min(1.0, bed_age / 0.5)
        bed_env *= math.exp(-max(0, bed_age - 1.1) * 0.62)

        chord = (
            math.sin(2 * math.pi * 523.25 * t) +
            0.65 * math.sin(2 * math.pi * 659.25 * t) +
            0.55 * math.sin(2 * math.pi * 783.99 * t) +
            0.30 * math.sin(2 * math.pi * 1046.5 * t)
        )

        sound += 0.10 * bed_env * chord

    # Dissolvenza finale
    fade = min(1.0, t / 0.025, (duration - t) / 0.45)

    samples.append(sound * max(0.0, fade))

save_wav(OUTPUT, samples)
print(f"Creato {OUTPUT}")