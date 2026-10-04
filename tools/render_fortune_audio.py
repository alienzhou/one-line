#!/usr/bin/env python3
"""Export the firmware's original sound renderer as reproducible audition WAVs."""
import ctypes
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import wave

ROOT = Path(__file__).resolve().parents[1]
RATE = 16000


def main():
    destination = ROOT / 'assets/music'
    destination.mkdir(parents=True, exist_ok=True)
    manifest = {}
    with tempfile.TemporaryDirectory(prefix='fortune-sound-') as temp:
        library = Path(temp) / ('renderer.dylib' if sys.platform == 'darwin' else 'renderer.so')
        subprocess.run([os.environ.get('CC', 'cc'), '-O2', '-std=c11', '-shared', '-fPIC',
                        str(ROOT / 'main/fortune_sound.c'), '-o', str(library)], check=True)
        renderer = ctypes.CDLL(str(library))
        renderer.fortune_sound_samples.argtypes = [ctypes.c_int]
        renderer.fortune_sound_samples.restype = ctypes.c_uint32
        renderer.fortune_sound_render.argtypes = [ctypes.c_int, ctypes.c_uint,
            ctypes.c_uint32, ctypes.POINTER(ctypes.c_int16), ctypes.c_size_t]
        renderer.fortune_sound_render.restype = ctypes.c_size_t

        def cue(effect, variant=0):
            count = renderer.fortune_sound_samples(effect)
            pcm = (ctypes.c_int16 * count)()
            assert renderer.fortune_sound_render(effect, variant, 0, pcm, count) == count
            return bytes(pcm)

        def export(name, pcm):
            path = destination / name
            with wave.open(str(path), 'wb') as output:
                output.setnchannels(1)
                output.setsampwidth(2)
                output.setframerate(RATE)
                output.writeframes(pcm)
            manifest[name] = {'sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
                              'frames': len(pcm) // 2, 'sample_rate': RATE}

        opening = []
        for variant in range(4):
            draw = cue(1, variant) + bytes(RATE // 10 * 2) + cue(2, variant)
            export(f'fortune-draw-{variant + 1}.wav', draw)
            opening.append(draw)
        keep = cue(3)
        skin = cue(4)
        export('fortune-keep.wav', keep)
        export('fortune-skin.wav', skin)
        export('fortune-audition.wav', opening[0] + bytes(RATE // 2 * 2) + keep +
               bytes(RATE // 2 * 2) + skin)
    (destination / 'fortune-audio.json').write_text(json.dumps({
        'render_status': 'completed', 'source': 'main/fortune_sound.c',
        'format': '16000 Hz, signed 16-bit mono PCM WAV', 'files': manifest,
        'note': 'Actual synthesized samples, without device speaker or codec coloration.'
    }, indent=2) + '\n')
    print(f'COMPLETE: {len(manifest)} finished WAV previews from the firmware renderer.')


if __name__ == '__main__':
    main()
