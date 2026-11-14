"""Verify room hardware checks and game failure UI without modifying SD assets.

Requires debug diagnostics and the existing 3-t_loop game on SD. User statistics
are isolated in the established diagnostic sandbox and original preferences are
not rewritten. Missing I2C uses address 0x27; disconnect is explicitly injected.
Physical DS2401 success/unplug tests require the separately documented hardware.
"""
import argparse
import json
from pathlib import Path
import re
import time

from ui_stability import StabilityProbe
from ui_settings import preferences, labels, confirm_players, begin_statistics_sandbox, end_statistics_sandbox


def records(probe):
    return next(json.loads(row.split('@STATS ', 1)[1]) for row in probe.command('ui stats-data') if '@STATS ' in row)


def wait_labels(probe, predicate, timeout=25):
    deadline = time.monotonic()+timeout
    while time.monotonic() < deadline:
        values = labels(probe)
        if predicate(values): return values
        time.sleep(.1)
    raise AssertionError('Expected hardware/game state not displayed: '+repr(values))


def run(args):
    probe = StabilityProbe(args.port, args.output)
    original = sandbox = None
    try:
        original = preferences(probe)
        probe.command('ui wifi-log off')
        probe.command('ui audio-master 0')
        probe.command('ui home')
        result = probe.command('ui hardware-test')
        assert any(re.search(r'@HWTEST passed=\d+ failed=0', row) for row in result), result
        print(next(row for row in result if '@HWTEST' in row), flush=True)
        if args.core_only: return
        sandbox = begin_statistics_sandbox(probe)
        for language in ('it', 'en'):
            probe.command('ui preview-language '+language)
            baseline = records(probe)
            for mode, message in [('missing', 'MCP23017'), ('invalid', 'GPIO 10')]:
                beginning = len(probe.log)
                probe.command('ui hardware-game '+mode)
                values = wait_labels(probe, lambda values: any(message in text for text in values))
                scope = probe.log[beginning:]
                entered = next(int(match[1]) for row in scope if (match := re.search(r'\[(\d+)\].*Entering GameScene:', row)))
                rejected = next(int(match[1]) for row in scope if (match := re.search(r'\[(\d+)\].*Hardware check failed:', row)))
                assert rejected-entered < 2000, f'Hardware rejection delayed {rejected-entered} ms'
                assert not any('Game assets prepared:' in row for row in scope), 'Failed hardware must be rejected before asset preload'
                assert not any(text in ('Tempo scaduto', 'Time expired') for text in values), values
                states = probe.command('ui hardware-state')
                assert any('lua=0' in row for row in states if '@HWSTATE ' in row), states
                assert records(probe) == baseline, 'Rejected hardware must not start a session'
                probe.capture(f'{language}-{mode}')
                print(f'PASS {language}/{mode}: rejected before assets in {rejected-entered} ms', flush=True)
                if mode == 'missing':
                    probe.click('Riprova' if language == 'it' else 'Retry')
                    wait_labels(probe, lambda values: any(message in text for text in values))
                    assert records(probe) == baseline, 'Failed retry must not change statistics'
                probe.click('Indietro' if language == 'it' else 'Back')
                probe.command('ui home')
        probe.command('ui preview-language '+('it' if original[1].startswith('it') else 'en'))
        language = 'it' if original[1].startswith('it') else 'en'
        probe.command('ui hardware-game working')
        wait_labels(probe, lambda values: any(re.fullmatch(r'\d{2}:\d{2}:\d{2}', text) for text in values))
        probe.click('Avvia' if language == 'it' else 'Start')
        confirm_players(probe)
        deadline = time.monotonic()+10
        while time.monotonic() < deadline and records(probe)['s'][0] == baseline['s'][0]: time.sleep(.1)
        started = records(probe)
        assert started['s'][0] == baseline['s'][0]+1, (baseline, started)
        probe.command('ui hardware-fault')
        wait_labels(probe, lambda values: any('Collegamento interrotto' in text or 'Connection lost' in text for text in values))
        states = probe.command('ui hardware-state')
        assert any('lua=0' in row for row in states if '@HWSTATE ' in row), states
        stopped = records(probe)
        assert stopped['s'][1:3] == started['s'][1:3], 'Hardware failure must not count as a win/loss'
        probe.capture('disconnected')
        probe.click('Riprova' if language == 'it' else 'Retry')
        wait_labels(probe, lambda values: any(re.fullmatch(r'\d{2}:\d{2}:\d{2}', text) for text in values))
        assert records(probe) == stopped, 'Retry loads a game without automatically starting a new session'
        probe.command('ui home')
        assert preferences(probe) == original, 'User preferences changed'
        print('PASS missing/invalid hardware blocks Lua and statistics, retry/back work in IT/EN; disconnect stops without loss', flush=True)
    finally:
        try:
            probe.command('ui home')
            if sandbox is not None: end_statistics_sandbox(probe, sandbox)
            if original: probe.command('ui preview-language '+('it' if original[1].startswith('it') else 'en'))
            probe.command('ui audio-master 100')
            probe.command('ui wifi-log on')
        finally:
            probe.close('session')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', default='COM3')
    parser.add_argument('--output', type=Path, default=Path('.pio/ui-hardware'))
    parser.add_argument('--core-only', action='store_true', help='Run checker/layout tests without starting a game')
    run(parser.parse_args())
