import math
import random
import struct
import wave
from pathlib import Path


SAMPLE_RATE = 44100
MASTER_GAIN = 0.72


def clamp(value, low=-1.0, high=1.0):
    return max(low, min(high, value))


def midi_to_freq(note):
    return 440.0 * (2.0 ** ((note - 69) / 12.0))


def sine(freq, t):
    return math.sin(2.0 * math.pi * freq * t)


def triangle(freq, t):
    cycle = (t * freq) % 1.0
    return 4.0 * abs(cycle - 0.5) - 1.0


def square(freq, t, duty=0.5):
    return 1.0 if (t * freq) % 1.0 < duty else -1.0


def noise(_freq, _t):
    return random.uniform(-1.0, 1.0)


def envelope(t, attack, decay, sustain, release, total):
    if t < 0.0 or t > total:
        return 0.0
    if t < attack:
        return t / max(attack, 1e-6)
    if t < attack + decay:
        progress = (t - attack) / max(decay, 1e-6)
        return 1.0 + (sustain - 1.0) * progress
    if t < total - release:
        return sustain
    progress = (t - (total - release)) / max(release, 1e-6)
    return sustain * (1.0 - progress)


def pan_stereo(sample, pan):
    left = sample * math.sqrt((1.0 - pan) * 0.5)
    right = sample * math.sqrt((1.0 + pan) * 0.5)
    return left, right


def add_note(buffer_left, buffer_right, start, duration, freq, wave_fn, volume, pan=0.0,
             attack=0.01, decay=0.04, sustain=0.7, release=0.08, vibrato=0.0, duty=0.5):
    start_index = int(start * SAMPLE_RATE)
    end_index = min(len(buffer_left), start_index + int(duration * SAMPLE_RATE))
    for index in range(start_index, end_index):
        t = (index - start_index) / SAMPLE_RATE
        env = envelope(t, attack, decay, sustain, release, duration)
        current_freq = freq * (1.0 + vibrato * math.sin(2.0 * math.pi * 6.0 * t))
        if wave_fn is square:
            sample = wave_fn(current_freq, t, duty)
        else:
            sample = wave_fn(current_freq, t)
        sample *= env * volume
        left, right = pan_stereo(sample, pan)
        buffer_left[index] += left
        buffer_right[index] += right


def add_drum(buffer_left, buffer_right, start, duration, base_freq, volume, pan=0.0, sparkle=False):
    start_index = int(start * SAMPLE_RATE)
    end_index = min(len(buffer_left), start_index + int(duration * SAMPLE_RATE))
    for index in range(start_index, end_index):
        t = (index - start_index) / SAMPLE_RATE
        env = max(0.0, 1.0 - t / max(duration, 1e-6))
        sweep = base_freq * (1.0 - 0.7 * min(1.0, t / max(duration, 1e-6)))
        sample = 0.55 * sine(sweep, t) + 0.35 * noise(0.0, t)
        if sparkle:
            sample += 0.25 * square(base_freq * 2.5, t, 0.2)
        sample *= (env ** 2) * volume
        left, right = pan_stereo(sample, pan)
        buffer_left[index] += left
        buffer_right[index] += right


def normalize_to_pcm(left, right):
    peak = max(max(abs(v) for v in left), max(abs(v) for v in right), 1e-6)
    scale = MASTER_GAIN / peak
    frames = bytearray()
    for l, r in zip(left, right):
        lv = int(clamp(l * scale) * 32767)
        rv = int(clamp(r * scale) * 32767)
        frames.extend(struct.pack("<hh", lv, rv))
    return frames


def write_wav(path, left, right):
    path.parent.mkdir(parents=True, exist_ok=True)
    frames = normalize_to_pcm(left, right)
    with wave.open(str(path), "wb") as wav_file:
        wav_file.setnchannels(2)
        wav_file.setsampwidth(2)
        wav_file.setframerate(SAMPLE_RATE)
        wav_file.writeframes(frames)


def make_music(duration):
    count = int(duration * SAMPLE_RATE)
    return [0.0] * count, [0.0] * count


def build_menu_theme(path):
    bpm = 96
    beat = 60.0 / bpm
    bar = beat * 4.0
    duration = bar * 8.0
    left, right = make_music(duration)

    chord_roots = [60, 65, 67, 60, 64, 67, 65, 60]
    melody = [
        (0.0, 1.0, 79), (1.0, 1.0, 81), (2.0, 1.0, 84), (3.0, 1.0, 81),
        (4.0, 1.0, 77), (5.0, 1.0, 79), (6.0, 1.0, 81), (7.0, 1.0, 79),
        (8.0, 1.0, 81), (9.0, 1.0, 84), (10.0, 1.0, 86), (11.0, 1.0, 84),
        (12.0, 1.0, 79), (13.0, 1.0, 81), (14.0, 1.0, 84), (15.0, 1.0, 79),
    ]

    for bar_index, root in enumerate(chord_roots):
        start = bar_index * bar
        chord = [root, root + 4, root + 7]
        bass_pattern = [root - 24, root - 12, root - 17, root - 12]

        for beat_index, bass_note in enumerate(bass_pattern):
            add_note(left, right, start + beat * beat_index, beat * 0.75, midi_to_freq(bass_note),
                     triangle, 0.038, pan=-0.03, attack=0.02, decay=0.08, sustain=0.45, release=0.14)

        for beat_index in range(4):
            arpeggio_note = chord[beat_index % len(chord)] + 12
            add_note(left, right, start + beat * beat_index, beat * 0.80, midi_to_freq(arpeggio_note),
                     sine, 0.032, pan=-0.12 if beat_index % 2 == 0 else 0.12,
                     attack=0.02, decay=0.10, sustain=0.55, release=0.18)
            add_note(left, right, start + beat * beat_index + beat * 0.33, beat * 0.55, midi_to_freq(arpeggio_note + 12),
                     triangle, 0.016, pan=0.18 if beat_index % 2 == 0 else -0.18,
                     attack=0.01, decay=0.05, sustain=0.35, release=0.10)
            add_note(left, right, start + beat * beat_index + beat * 0.72, beat * 0.22, midi_to_freq(arpeggio_note + 24),
                     sine, 0.010, pan=0.24 if beat_index % 2 == 0 else -0.24,
                     attack=0.005, decay=0.04, sustain=0.25, release=0.06)

    for start_beat, length_beats, note in melody:
        add_note(left, right, start_beat * beat, length_beats * beat, midi_to_freq(note),
                 triangle, 0.10, pan=0.08, attack=0.02, decay=0.08, sustain=0.72, release=0.16,
                 vibrato=0.002)
        add_note(left, right, start_beat * beat, length_beats * beat, midi_to_freq(note + 12),
                 sine, 0.030, pan=0.22, attack=0.01, decay=0.05, sustain=0.34, release=0.10)
        add_note(left, right, start_beat * beat, length_beats * beat, midi_to_freq(note - 12),
                 sine, 0.012, pan=-0.08, attack=0.02, decay=0.08, sustain=0.24, release=0.12)
        add_note(left, right, start_beat * beat + beat * 0.12, length_beats * beat * 0.35, midi_to_freq(note + 24),
                 sine, 0.014, pan=0.28, attack=0.003, decay=0.04, sustain=0.18, release=0.05)

    write_wav(path, left, right)


def build_mission_theme(path):
    bpm = 110
    beat = 60.0 / bpm
    bar = beat * 4.0
    duration = bar * 8.0
    left, right = make_music(duration)

    phrases = [
        (0.0, 0.5, 76), (0.5, 0.5, 79), (1.0, 1.0, 81), (2.0, 1.0, 84),
        (4.0, 0.5, 77), (4.5, 0.5, 81), (5.0, 1.0, 84), (6.0, 1.0, 86),
        (8.0, 0.5, 79), (8.5, 0.5, 81), (9.0, 1.0, 84), (10.0, 1.0, 88),
        (12.0, 0.5, 81), (12.5, 0.5, 84), (13.0, 1.0, 86), (14.0, 1.0, 84),
    ]
    root_cycle = [57, 60, 64, 65, 57, 62, 65, 69]

    for bar_index, root in enumerate(root_cycle):
        start = bar_index * bar
        chord = [root, root + 3, root + 7]
        bass_pattern = [root - 24, root - 12, root - 17, root - 12]

        for beat_index, bass_note in enumerate(bass_pattern):
            add_note(left, right, start + beat * beat_index, beat * 0.72, midi_to_freq(bass_note),
                     triangle, 0.042, pan=-0.03, attack=0.02, decay=0.08, sustain=0.44, release=0.14)

        for beat_index in range(4):
            arpeggio_note = chord[(beat_index + 1) % len(chord)] + 12
            add_note(left, right, start + beat * beat_index, beat * 0.78, midi_to_freq(arpeggio_note),
                     sine, 0.030, pan=-0.10 if beat_index % 2 == 0 else 0.10,
                     attack=0.02, decay=0.08, sustain=0.52, release=0.16)
            add_note(left, right, start + beat * beat_index + beat * 0.28, beat * 0.48, midi_to_freq(arpeggio_note + 12),
                     sine, 0.012, pan=0.16 if beat_index % 2 == 0 else -0.16,
                     attack=0.01, decay=0.04, sustain=0.30, release=0.08)
            add_note(left, right, start + beat * beat_index + beat * 0.62, beat * 0.20, midi_to_freq(arpeggio_note + 24),
                     sine, 0.008, pan=0.22 if beat_index % 2 == 0 else -0.22,
                     attack=0.004, decay=0.03, sustain=0.16, release=0.05)

    for start_beat, length_beats, note in phrases:
        add_note(left, right, start_beat * beat, length_beats * beat, midi_to_freq(note),
                 triangle, 0.09, pan=0.08, attack=0.02, decay=0.08, sustain=0.72, release=0.16,
                 vibrato=0.002)
        add_note(left, right, start_beat * beat, length_beats * beat, midi_to_freq(note + 7),
                 sine, 0.025, pan=0.20, attack=0.01, decay=0.05, sustain=0.32, release=0.10)
        add_note(left, right, start_beat * beat, length_beats * beat, midi_to_freq(note - 12),
                 sine, 0.012, pan=-0.08, attack=0.02, decay=0.08, sustain=0.24, release=0.12)
        add_note(left, right, start_beat * beat + beat * 0.10, length_beats * beat * 0.28, midi_to_freq(note + 24),
                 sine, 0.012, pan=0.24, attack=0.003, decay=0.03, sustain=0.16, release=0.05)

    write_wav(path, left, right)


def write_effect(path, duration, renderer):
    samples = int(duration * SAMPLE_RATE)
    left = [0.0] * samples
    right = [0.0] * samples
    for index in range(samples):
        t = index / SAMPLE_RATE
        sample = renderer(t, duration)
        l, r = pan_stereo(sample, 0.0)
        left[index] = l
        right[index] = r
    write_wav(path, left, right)


def pop_renderer(base_freq, alt_freq, sparkle=False):
    def render(t, total):
        env = envelope(t, 0.004, 0.04, 0.12, 0.08, total)
        sample = 0.16 * sine(base_freq * (1.0 + 0.04 * math.sin(18 * t)), t)
        sample += 0.08 * triangle(alt_freq, t)
        sample += 0.05 * sine(base_freq * 1.5, t)
        if sparkle:
            sample += 0.035 * sine(base_freq * 3.0, t)
        return sample * env * 0.22
    return render


def whoosh_renderer():
    def render(t, total):
        env = envelope(t, 0.005, 0.04, 0.2, 0.12, total)
        freq = 220 + 420 * (t / total)
        sample = 0.13 * sine(freq, t) + 0.03 * noise(0.0, t)
        sample += 0.06 * sine(freq * 2.0, t)
        return sample * env * 0.20
    return render


def pickup_renderer():
    tones = [(0.00, 0.12, 84), (0.07, 0.12, 88), (0.15, 0.15, 91), (0.24, 0.16, 96)]

    def render(t, total):
        sample = 0.0
        for start, length, note in tones:
            local_t = t - start
            if 0.0 <= local_t <= length:
                env = envelope(local_t, 0.003, 0.03, 0.45, 0.05, length)
                sample += sine(midi_to_freq(note), local_t) * env * 0.30
                sample += sine(midi_to_freq(note + 12), local_t) * env * 0.12
        return sample
    return render


def bubble_boom_renderer(offset):
    def render(t, total):
        env = max(0.0, 1.0 - t / total)
        drop = 220.0 * (1.0 - 0.75 * min(1.0, t / total))
        sample = 0.18 * sine(drop + offset, t)
        sample += 0.10 * noise(0.0, t) * (env ** 1.4)
        sample += 0.05 * triangle((drop * 1.4) + 30.0, t)
        sample += 0.04 * sine((drop * 2.4) + 120.0, t)
        return sample * (env ** 2) * 0.34
    return render


def button_renderer():
    def render(t, total):
        env = envelope(t, 0.002, 0.03, 0.18, 0.04, total)
        sample = 0.42 * sine(1046.5, t) + 0.18 * sine(1568.0, t) + 0.06 * sine(2093.0, t)
        return sample * env * 0.24
    return render


def main():
    root = Path(__file__).resolve().parents[1]
    build_menu_theme(root / "Media" / "Music" / "CuteMenuTheme.wav")
    build_mission_theme(root / "Media" / "Music" / "CuteMissionTheme.wav")

    write_effect(root / "Media" / "Sound" / "CuteAlliedGunfire.wav", 0.12, pop_renderer(780.0, 1170.0, True))
    write_effect(root / "Media" / "Sound" / "CuteEnemyGunfire.wav", 0.11, pop_renderer(540.0, 810.0, False))
    write_effect(root / "Media" / "Sound" / "CuteExplosion1.wav", 0.40, bubble_boom_renderer(0.0))
    write_effect(root / "Media" / "Sound" / "CuteExplosion2.wav", 0.48, bubble_boom_renderer(35.0))
    write_effect(root / "Media" / "Sound" / "CuteLaunch.wav", 0.22, whoosh_renderer())
    write_effect(root / "Media" / "Sound" / "CuteCollectPickup.wav", 0.34, pickup_renderer())
    write_effect(root / "Media" / "Sound" / "CuteButton.wav", 0.10, button_renderer())


if __name__ == "__main__":
    random.seed(7)
    main()
