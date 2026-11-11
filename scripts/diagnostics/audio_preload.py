"""Measure preloaded GameView event sounds on the ESP32 (debug diagnostics).

Reads existing SD assets; no Lua/relays or preference writes. Playback uses 1%
master while testing and is restored on exit. FX must already be enabled.
"""
import argparse
from pathlib import Path
import re

from ui_settings import preferences
from ui_stability import StabilityProbe

FILES = {'clock': 'tickingclock.wav', 'resume': 'tickingclock.wav',
         'fail': 'lose.wav', 'victory': 'win.wav'}


def run(args):
    probe = StabilityProbe(args.port,args.output)
    preview = False
    original = None
    try:
        original = preferences(probe)
        assert original[2][2] and original[2][3], 'Enable sound effects before testing audible playback'
        result = probe.command('ui preload-test')
        assert any(re.search(r'@PRELOADTEST passed=\d+ failed=0',row) for row in result),result
        print(next(row for row in result if '@PRELOADTEST' in row),flush=True)
        start = len(probe.log)
        preview = True
        rows = probe.command('ui preload-preview')
        if not any('@PRELOADREADY' in row for row in rows):
            probe.read_until(lambda row: '@PRELOADREADY' in row,timeout=40)
        warm = probe.log[start:]
        assert any('Game audio prepared' in row and '(all ready)' in row for row in warm),warm
        for filename in {'tickingclock.wav','lose.wav','win.wav'}:
            assert any(f'Preload /.core/audio/{filename}: ready' in row for row in warm),warm
        print(next(row for row in warm if '@PRELOADREADY' in row),flush=True)
        start = len(probe.log)
        probe.command('ui preload-event pressure')
        if not any('Preload /.core/audio/main_theme.wav:' in row for row in probe.log[start:]):
            probe.read_until(lambda row: 'Preload /.core/audio/main_theme.wav:' in row,timeout=20)
        print('PASS cache pressure request processed before event playback',flush=True)
        for event in ('clock','pause','resume','fail','victory'):
            start = len(probe.log)
            rows = probe.command('ui preload-event '+event)
            if event == 'pause':
                continue
            expected = FILES[event]
            match = next((re.search(r'WAV started: /\.core/audio/(\w+\.wav) cache=(\w+) delay=(\d+) ms',row)
                          for row in rows if 'WAV started:' in row and expected in row),None)
            if not match:
                probe.read_until(lambda row: 'WAV started:' in row and expected in row,timeout=5)
                match = next((re.search(r'WAV started: /\.core/audio/(\w+\.wav) cache=(\w+) delay=(\d+) ms',row)
                              for row in probe.log[start:] if 'WAV started:' in row and expected in row),None)
            assert match and match[2] == 'hit' and int(match[3]) <= args.max_delay_ms,probe.log[start:]
            assert not any(f'Opened file: /.core/audio/{expected}' in row for row in probe.log[start:]),probe.log[start:]
            print(f'PASS {event}: cache hit, queued-to-mixer {match[3]} ms, no SD open',flush=True)
        probe.command('ui home'); preview = False
        assert preferences(probe) == original,'preferences changed during preload test'
    finally:
        if preview:
            probe.command('ui home')
        probe.close('session')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port',default='COM3')
    parser.add_argument('--output',type=Path,default=Path('.pio/audio-preload-check'))
    parser.add_argument('--max-delay-ms',type=int,default=50)
    run(parser.parse_args())
