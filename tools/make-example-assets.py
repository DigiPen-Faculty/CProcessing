"""Make the sprite sheet, music and sound effects the examples use.

    python tools/make-example-assets.py

Writes them to Processing_Empty/Assets, the Assets folder every template
package ships with. Everything is drawn or synthesized here, from nothing
but code, so the files are original and MIT licensed like the rest of
CProcessing. Run it again only to change them; the output is committed.

    SlimeSheet.png  8 frames of a bouncing slime, each 64x64, in one row
    Music.wav       an 8-second loop: drums, bass, chords and a melody
    Coin.wav        a two-note pickup chime
    Jump.wav        a rising blip
    Hit.wav         a short thump

Needs Pillow (pip install pillow) for the sprite sheet.
"""
import math
import random
import struct
import wave
from pathlib import Path

ASSETS = Path(__file__).resolve().parent.parent / "Processing_Empty" / "Assets"
RATE = 22050


# ---------------------------------------------------------------------------
# Sprite sheet

def make_slime_sheet(path, frames=8, size=64, scale=4):
    from PIL import Image, ImageDraw

    big = size * scale
    sheet = Image.new("RGBA", (size * frames, size), (0, 0, 0, 0))
    for f in range(frames):
        t = f / frames
        # One hop per cycle: squashed on the ground at t = 0, stretched and
        # highest at t = 0.5
        lift = math.sin(math.pi * t)                 # 0 on the ground, 1 at the top
        stretch = 1.0 + 0.22 * math.cos(2 * math.pi * t + math.pi)
        width = 0.62 * big / stretch ** 0.5
        height = 0.46 * big * stretch
        bottom = big * 0.92 - lift * big * 0.18
        cx = big / 2

        img = Image.new("RGBA", (big, big), (0, 0, 0, 0))
        d = ImageDraw.Draw(img)

        # shadow on the ground, smaller while in the air
        sw = big * 0.5 * (1.0 - 0.4 * lift)
        d.ellipse([cx - sw / 2, big * 0.9, cx + sw / 2, big * 0.97], fill=(0, 0, 0, 70))

        # body: a dark outline, the green body, and a light highlight
        top = bottom - height
        d.ellipse([cx - width / 2 - 6, top - 6, cx + width / 2 + 6, bottom + 6], fill=(20, 70, 30, 255))
        d.ellipse([cx - width / 2, top, cx + width / 2, bottom], fill=(80, 200, 90, 255))
        d.ellipse([cx - width * 0.32, top + height * 0.12, cx - width * 0.05, top + height * 0.34],
                  fill=(190, 250, 190, 255))

        # eyes, looking slightly up while in the air
        eye_y = top + height * 0.45 - lift * 6
        for side in (-1, 1):
            ex = cx + side * width * 0.18
            d.ellipse([ex - 18, eye_y - 22, ex + 18, eye_y + 22], fill=(255, 255, 255, 255))
            d.ellipse([ex - 9, eye_y - 14 - lift * 6, ex + 9, eye_y + 6 - lift * 6], fill=(20, 20, 30, 255))

        sheet.paste(img.resize((size, size), Image.LANCZOS), (f * size, 0))
    sheet.save(path)


# ---------------------------------------------------------------------------
# Sound

def note_hz(name):
    """'C4', 'F#3' and so on, to a frequency in Hz (A4 = 440)."""
    names = {"C": -9, "C#": -8, "D": -7, "D#": -6, "E": -5, "F": -4,
             "F#": -3, "G": -2, "G#": -1, "A": 0, "A#": 1, "B": 2}
    pitch, octave = name[:-1], int(name[-1])
    return 440.0 * 2 ** ((names[pitch] + 12 * (octave - 4)) / 12)


def triangle(phase):
    return 4 * abs(phase - math.floor(phase + 0.5)) - 1


def pulse(phase, duty):
    return 1.0 if (phase % 1.0) < duty else -1.0


def envelope(i, length, attack, release):
    """A linear fade in over `attack` samples and out over `release`."""
    if i < attack:
        return i / attack
    if i > length - release:
        return max(0.0, (length - i) / release)
    return 1.0


def add_tone(buf, start, seconds, hz, volume, wave_fn, release=0.02, decay=0.0):
    n = int(seconds * RATE)
    attack = int(0.004 * RATE)
    rel = int(release * RATE)
    for i in range(n):
        j = start + i
        if j >= len(buf):
            break
        amp = volume * envelope(i, n, attack, rel)
        if decay:
            amp *= math.exp(-decay * i / RATE)
        buf[j] += amp * wave_fn(hz * i / RATE)


def lowpass(buf, amount):
    out, y = [], 0.0
    for x in buf:
        y += amount * (x - y)
        out.append(y)
    return out


def write_wav(path, samples, peak=0.7):
    top = max(abs(s) for s in samples) or 1.0
    with wave.open(str(path), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(b"".join(struct.pack("<h", int(32767 * peak * s / top)) for s in samples))


def make_music(path):
    bpm = 120
    beat = 60 / bpm
    bars = 4
    length = int(bars * 4 * beat * RATE)
    drums = [0.0] * length
    bass = [0.0] * length
    chords = [0.0] * length
    melody = [0.0] * length
    noise = random.Random(7)

    progression = [("C3", ["C4", "E4", "G4"]), ("G2", ["B3", "D4", "G4"]),
                   ("A2", ["C4", "E4", "A4"]), ("F2", ["C4", "F4", "A4"])]
    tune = [  # (beat within the bar, beats long, note) for each bar
        [(0, 1, "E5"), (1, 1, "G5"), (2, 1.5, "C6"), (3.5, 0.5, "B5")],
        [(0, 1, "D5"), (1, 1, "G5"), (2, 2, "B5")],
        [(0, 1, "C5"), (1, 1, "E5"), (2, 1, "A5"), (3, 1, "G5")],
        [(0, 1.5, "F5"), (1.5, 0.5, "E5"), (2, 2, "C5")],
    ]

    for bar, (root, chord) in enumerate(progression):
        bar_start = bar * 4 * beat
        for step in range(8):  # eighth notes
            t = bar_start + step * beat / 2
            s = int(t * RATE)
            # kick on 1 and 3, hi-hat on every eighth, snare-ish noise on 2 and 4
            if step in (0, 4):
                n = int(0.18 * RATE)
                for i in range(n):
                    if s + i < length:
                        f = 50 + 90 * math.exp(-i / (0.03 * RATE))
                        drums[s + i] += 0.9 * math.exp(-i / (0.06 * RATE)) * math.sin(2 * math.pi * f * i / RATE)
            if step in (2, 6):
                n = int(0.12 * RATE)
                for i in range(n):
                    if s + i < length:
                        drums[s + i] += 0.35 * math.exp(-i / (0.03 * RATE)) * (noise.random() * 2 - 1)
            n = int(0.03 * RATE)
            for i in range(n):
                if s + i < length:
                    drums[s + i] += 0.12 * math.exp(-i / (0.006 * RATE)) * (noise.random() * 2 - 1)
            # bass: the root, an octave up on the off-beats
            hz = note_hz(root) * (2 if step % 2 else 1)
            add_tone(bass, s, beat / 2 * 0.9, hz, 0.45, triangle, release=0.03)
            # chords: an arpeggio up the chord
            add_tone(chords, s, beat / 2 * 0.8, note_hz(chord[step % 3]), 0.12,
                     lambda p: pulse(p, 0.25), release=0.05, decay=4.0)
        for start, beats, name in tune[bar]:
            s = int((bar_start + start * beat) * RATE)
            add_tone(melody, s, beats * beat * 0.92, note_hz(name), 0.22,
                     lambda p: 0.6 * pulse(p, 0.5) + 0.4 * triangle(p), release=0.06, decay=1.5)

    chords = lowpass(chords, 0.25)
    melody = lowpass(melody, 0.35)
    mix = [d + b + c + m for d, b, c, m in zip(drums, bass, chords, melody)]
    write_wav(path, mix)


def make_coin(path):
    buf = [0.0] * int(0.35 * RATE)
    add_tone(buf, 0, 0.07, note_hz("B5"), 0.5, lambda p: pulse(p, 0.5), release=0.01)
    add_tone(buf, int(0.07 * RATE), 0.28, note_hz("E6"), 0.5, lambda p: pulse(p, 0.5), release=0.05, decay=9.0)
    write_wav(path, lowpass(buf, 0.5))


def make_jump(path):
    n = int(0.25 * RATE)
    buf, phase = [], 0.0
    for i in range(n):
        hz = 280 + 520 * (i / n) ** 0.7
        phase += hz / RATE
        buf.append(envelope(i, n, 60, int(0.08 * RATE)) * pulse(phase, 0.25))
    write_wav(path, lowpass(buf, 0.4))


def make_hit(path):
    n = int(0.25 * RATE)
    noise = random.Random(3)
    buf = []
    for i in range(n):
        thump = math.sin(2 * math.pi * (60 + 120 * math.exp(-i / (0.02 * RATE))) * i / RATE)
        crack = noise.random() * 2 - 1
        buf.append(math.exp(-i / (0.05 * RATE)) * (0.7 * thump + 0.5 * crack * math.exp(-i / (0.015 * RATE))))
    write_wav(path, lowpass(buf, 0.6))


if __name__ == "__main__":
    ASSETS.mkdir(exist_ok=True)
    make_slime_sheet(ASSETS / "SlimeSheet.png")
    make_music(ASSETS / "Music.wav")
    make_coin(ASSETS / "Coin.wav")
    make_jump(ASSETS / "Jump.wav")
    make_hit(ASSETS / "Hit.wav")
    for name in ("SlimeSheet.png", "Music.wav", "Coin.wav", "Jump.wav", "Hit.wav"):
        print(f"{ASSETS / name}  {(ASSETS / name).stat().st_size} bytes")
