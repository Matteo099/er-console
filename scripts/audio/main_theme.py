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

OUTPUT = "main_theme" + EXTENSIONS

DURATION = 32.0

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

    size = os.path.getsize(filename)

    if size > MAX_BYTES:
        os.remove(filename)
        raise ValueError("File audio superiore a 4 MB")

# Progressione armonica originale
# Re minore e armonie sospese
progression = [
    (146.83, 174.61, 220.00),
    (130.81, 174.61, 196.00),
    (110.00, 164.81, 220.00),
    (130.81, 155.56, 196.00)
]

# Melodia principale
melody = [
    (2.0, 587.33),
    (5.5, 523.25),
    (8.0, 440.00),
    (11.5, 493.88),
    (14.0, 587.33),
    (18.0, 659.25),
    (21.5, 587.33),
    (24.0, 523.25),
    (27.5, 440.00)
]

samples = []

total_samples = int(SAMPLE_RATE * DURATION)

for n in range(total_samples):

    t = n / SAMPLE_RATE

    # Accordo attivo
    bar = int(t // 8) % 4
    chord = progression[bar]

    # ---------------------------------
    # 1. Tappeto armonico sintetico
    # ---------------------------------

    pad_envelope = 0.72 + 0.08 * math.sin(
        2 * math.pi * t / 11.0
    )

    pad = 0.0

    for j, frequency in enumerate(chord):

        phase = 2 * math.pi * frequency * t

        modulation = 0.12 * math.sin(
            2 * math.pi * t / 7.0
        )

        tone = (
            math.sin(phase + modulation) +
            0.22 * math.sin(2 * phase)
        )

        pad += (0.075 / (j + 1)) * tone

    pad *= pad_envelope

    # ---------------------------------
    # 2. Pulsazione grave
    # ---------------------------------

    pulse_phase = t % 2.0

    pulse_envelope = math.exp(-pulse_phase * 2.2)

    bass_frequency = chord[0] / 2

    bass = (
        0.13 *
        pulse_envelope *
        math.sin(2 * math.pi * bass_frequency * t)
    )

    # ---------------------------------
    # 3. Melodia elettronica
    # ---------------------------------

    bell = 0.0

    for start, frequency in melody:

        age = t - start

        if 0 <= age < 4.8:

            envelope = math.exp(-age * 1.05)
            envelope *= min(1.0, age / 0.05)

            tone = (
                math.sin(2 * math.pi * frequency * age) +
                0.25 * math.sin(
                    2 * math.pi * frequency * 2.01 * age
                )
            )

            bell += 0.075 * envelope * tone

    # ---------------------------------
    # 4. Effetto shimmer tecnologico
    # ---------------------------------

    shimmer = (
        0.018 *
        math.sin(
            2 * math.pi * 1320 * t +
            0.4 * math.sin(2 * math.pi * t / 9)
        )
    )

    shimmer *= (
        0.65 +
        0.35 * math.sin(2 * math.pi * t / 6.5)
    )

    # ---------------------------------
    # 5. Mix finale
    # ---------------------------------

    sound = pad + bass + bell + shimmer

    samples.append(sound)

# ---------------------------------
# Crossfade per il loop musicale
# ---------------------------------

fade_length = int(SAMPLE_RATE * 1.2)

for i in range(fade_length):

    alpha = i / fade_length

    end_index = total_samples - fade_length + i

    start_value = samples[i]
    end_value = samples[end_index]

    samples[end_index] = (
        end_value * (1 - alpha) +
        start_value * alpha
    )

save_wav(OUTPUT, samples)

print(f"Creato {OUTPUT}")
print(f"Durata: {DURATION} secondi")
print(f"Dimensione: {os.path.getsize(OUTPUT) / 1024:.1f} KB")