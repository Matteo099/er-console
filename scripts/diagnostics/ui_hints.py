"""Check Lua hint deadlines, the journal and auto-open on the real TFT.

Requires diagnostic firmware. Preview scenes do not start games, operate room
hardware or write statistics; original language/audio preferences are retained.
"""
import argparse
from pathlib import Path
import re
import time

from ui_settings import labels, preferences
from ui_stability import StabilityProbe


def state(probe):
    rows = probe.command('ui hints-state')
    match = next(re.search(r'@HINTS count=(\d+) active=(\d+) state=(\d+) reading=(\d+) viewed=(\d+)', row)
                 for row in rows if '@HINTS count=' in row)
    return tuple(map(int, match.groups()))


def run(args):
    probe = StabilityProbe(args.port, args.output)
    original = None
    try:
        original = preferences(probe)
        probe.command('ui audio-master 0')
        probe.command('ui wifi-log off')
        probe.command('ui home')
        rows = probe.command('ui hints-test')
        result = next(row for row in rows if '@HINTTEST ' in row)
        assert re.search(r'passed=\d+ failed=0', result), rows
        print(result, flush=True)
        for language in ('it', 'en'):
            probe.command('ui preview-language ' + language)
            title = 'Osserva la tavola' if language == 'it' else 'Inspect the board'
            probe.command('ui hints-preview locked')
            assert state(probe)[0] == 0
            assert title not in labels(probe), 'A locked hint was exposed'
            assert any('Nessun indizio' in row or 'No hints unlocked' in row for row in labels(probe))
            probe.capture(language + '-locked')

            probe.command('ui hints-preview')
            deadline = time.monotonic() + 10
            while time.monotonic() < deadline:
                current = state(probe)
                if current == (1, 0, 4, 1, 1):
                    break
                time.sleep(.1)
            assert current == (1, 0, 4, 1, 1), current
            assert title in labels(probe), 'New hint did not open automatically'
            probe.capture(language + '-auto-open')
            probe.click('\uf00b')
            assert state(probe)[2] == 6, 'Reader did not return to journal'
            probe.capture(language + '-journal')
            probe.click(title)
            assert state(probe)[-1] == 1, 'Reopening a hint counted it twice'

            probe.command('ui hints-preview list')
            assert state(probe)[0] == 8
            probe.capture(language + '-list')
            for offset in (100, 200, 300, 400):
                probe.command(f'ui scrollat 5 120 {offset}')
            last = 'Indizio 8' if language == 'it' else 'Hint 8'
            probe.click(last)
            assert last in labels(probe)
            assert state(probe)[-1] == 1
            probe.capture(language + '-last-hint')
            probe.click('\uf00b')
            probe.click('Oggetti' if language == 'it' else 'Objects')
            assert state(probe)[3] == 0
            probe.capture(language + '-objects-empty')
            probe.click('Indietro' if language == 'it' else 'Back')
            assert state(probe)[2] == 1
            print('PASS ' + language + ': locked list, auto-open, journal tabs, scrolling and unique hint usage', flush=True)
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
    parser.add_argument('--output', type=Path, default=Path('.pio/hints'))
    run(parser.parse_args())
