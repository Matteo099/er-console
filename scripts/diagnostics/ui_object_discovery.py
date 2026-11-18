"""Check item discovery, Lua dispatch and the real TFT animation without games/NVS writes."""
import argparse
from pathlib import Path
import re
import time

from ui_stability import StabilityProbe
from ui_settings import preferences


def state(probe):
    rows = probe.command('ui discovery-state')
    match = next(re.search(r'@DISCOVERY count=(\d+) active=(\d+)', row)
                 for row in rows if '@DISCOVERY count=' in row)
    return tuple(map(int, match.groups()))


def run(args):
    probe = StabilityProbe(args.port, args.output)
    original = None
    try:
        original = preferences(probe)
        probe.command('ui audio-master 0')
        probe.command('ui wifi-log off')
        probe.command('ui home')
        rows = probe.command('ui discovery-test')
        result = next(row for row in rows if '@DISCOVERYTEST ' in row)
        assert re.search(r'passed=\d+ failed=0', result), rows
        print(result, flush=True)
        for language in ('it', 'en'):
            probe.command('ui preview-language ' + language)
            for stage in ('reveal', 'flight', 'fallback'):
                probe.command('ui discovery-preview ' + stage)
                assert state(probe) == (0, 1), (language, stage)
                probe.capture(language + '-' + stage)
            probe.command('ui discovery-preview')
            deadline = time.monotonic() + 8
            while time.monotonic() < deadline and state(probe) != (1, 0):
                time.sleep(.1)
            assert state(probe) == (1, 0), 'First item did not land'
            probe.command('ui discovery-add')
            deadline = time.monotonic() + 8
            while time.monotonic() < deadline and state(probe) != (2, 0):
                time.sleep(.1)
            assert state(probe) == (2, 0), 'Queued item did not land'
            probe.capture(language + '-collected')
            print('PASS ' + language + ': reveal/flight/fallback and two sequential landings', flush=True)
        probe.command('ui home')
        probe.command('ui preview-language ' + ('it' if original[1].startswith('it') else 'en'))
        assert preferences(probe) == original
    finally:
        try:
            probe.command('ui home')
            if original:
                probe.command('ui preview-language ' + ('it' if original[1].startswith('it') else 'en'))
            probe.command('ui audio-master 100')
            probe.command('ui wifi-log on')
        finally:
            probe.close('session')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', default='COM4')
    parser.add_argument('--output', type=Path, default=Path('.pio/object-discovery'))
    run(parser.parse_args())
