"""Check the pure PCM mixer and isolated control/Lua queues on debug firmware.

Requires UI_DIAGNOSTICS_ENABLED=1. Tests use synthetic samples and private queues;
they do not write SD/preferences, run games, or send test samples to the amplifier.
"""
import argparse
from pathlib import Path
import re

from ui_stability import StabilityProbe


def run(args):
    probe = StabilityProbe(args.port, args.output)
    try:
        probe.command('ui quiet on')
        result = probe.command('ui audio-test')
        for row in result:
            if '@AUDIO' in row:
                print(row, flush=True)
        summary = next((re.search(r'@AUDIOTEST passed=(\d+) failed=(\d+)', row) for row in result if '@AUDIOTEST ' in row), None)
        assert summary and int(summary[1]) > 0 and int(summary[2]) == 0, result
        result = probe.command('ui selftest')
        assert any(re.search(r'@SELFTEST passed=\d+ failed=0', row) for row in result), result
        print(next(row for row in result if '@SELFTEST' in row), flush=True)
    finally:
        probe.command('ui quiet off')
        probe.close('session')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', default='COM3')
    parser.add_argument('--output', type=Path, default=Path('.pio/audio-check'))
    run(parser.parse_args())
