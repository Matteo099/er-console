"""Exercise the real one-minute Loop game through the TFT Scene/View flow.

The supplied Loop game only logs updates and disables every input subsystem.
Runs its full countdown; master is temporarily 1%, and preferences are preserved.
Requires UI_DIAGNOSTICS_ENABLED=1 and existing game/audio files on the SD.
"""
import argparse
from pathlib import Path
import re

from ui_settings import labels, preferences, confirm_players, begin_statistics_sandbox, end_statistics_sandbox
from ui_stability import StabilityProbe


def started(rows, filename):
    return next((re.search(r'WAV started: /\.core/audio/\w+\.wav cache=(\w+) delay=(\d+) ms',row)
                 for row in rows if 'WAV started:' in row and filename in row),None)


def run(args):
    probe = StabilityProbe(args.port,args.output)
    original = None
    original_stats = None
    try:
        original = preferences(probe)
        original_stats=begin_statistics_sandbox(probe)
        assert original[2][2] and original[2][3], 'Sound effects must be enabled'
        probe.command('ui audio-master 1')
        probe.command('ui games')
        for _ in range(5):
            if any('Test - Ciclo' in text or 'Test - Loop' in text for text in labels(probe)):
                break
            probe.click('\uf054')
        else:
            raise AssertionError('Loop game not found in catalog')
        start = len(probe.log)
        caption = 'Avvia' if original[1].startswith('it') else 'Start'
        probe.click(caption)
        if not any('Game loaded: 3-t_loop' in row for row in probe.log[start:]):
            probe.read_until(lambda row: 'Game loaded: 3-t_loop' in row,timeout=35)
        preparation = probe.log[start:]
        assert any('Game audio prepared' in row and '(all ready)' in row for row in preparation),preparation
        print('PASS real GameScene stays in preparation until all sounds are ready',flush=True)
        start = len(probe.log)
        probe.click(caption)
        confirm_players(probe)
        if not started(probe.log[start:],'tickingclock.wav'):
            probe.read_until(lambda row: 'WAV started:' in row and 'tickingclock.wav' in row,timeout=5)
        tick = started(probe.log[start:],'tickingclock.wav')
        assert tick and tick[1] == 'hit' and int(tick[2]) <= 50,probe.log[start:]
        print(f'PASS real one-minute countdown: cache hit, {tick[2]} ms; waiting for natural expiry',flush=True)
        if not started(probe.log[start:],'lose.wav'):
            probe.read_until(lambda row: 'WAV started:' in row and 'lose.wav' in row,timeout=70)
        failure = started(probe.log[start:],'lose.wav')
        assert failure and failure[1] == 'hit' and int(failure[2]) <= 50,probe.log[start:]
        print(f'PASS natural game failure: cache hit, {failure[2]} ms',flush=True)
        assert preferences(probe) == original,'preferences changed'
    finally:
        try:
            probe.command('ui home')
            if original_stats is not None: end_statistics_sandbox(probe,original_stats)
            probe.command('ui audio-master 100')
        finally:
            probe.close('session')


if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port',default='COM3')
    parser.add_argument('--output',type=Path,default=Path('.pio/audio-preload-game'))
    run(parser.parse_args())
