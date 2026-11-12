"""Verify player selector taps and holds through real LVGL input dispatch.

Requires debug firmware with UI_DIAGNOSTICS_ENABLED=1 and a free serial port.
Runs isolated tests, then opens a selector preview on the physical display.
Never starts games or changes preferences/statistics; returns to the home screen.
"""
import argparse
from pathlib import Path
import re
import time

from ui_stability import StabilityProbe
from ui_settings import labels, preferences


def send(probe, command):
    probe.port.write((command + '\n').encode())
    return probe.read_until(lambda row: '@UI DONE ' + command in row)


def touch(probe, point, duration):
    x, y = point
    send(probe, f'ui touch {x} {y} 1')
    time.sleep(duration)
    send(probe, f'ui touch {x} {y} 0')
    time.sleep(.12)


def count(probe):
    values = [int(text) for text in labels(probe) if re.fullmatch(r'\d+', text)]
    assert len(values) == 1, values
    return values[0]


def run(args):
    probe = StabilityProbe(args.port, args.output)
    try:
        probe.command('ui wifi-log off')
        original = preferences(probe)
        statistics = next(row.split('@STATS ', 1)[1] for row in probe.command('ui stats-data')
                          if '@STATS ' in row)
        probe.command('ui quiet on')
        result = probe.command('ui players-test')
        for row in result:
            if '@PLAYER' in row:
                print(row, flush=True)
        summary = next((re.search(r'@PLAYERTEST passed=(\d+) failed=(\d+)', row)
                        for row in result if '@PLAYERTEST ' in row), None)
        assert summary and int(summary[1]) > 0 and int(summary[2]) == 0, result
        if args.native_only:
            return
        probe.command('ui players')
        # The decrease control starts disabled at one player; retain its
        # coordinates so it can be used after the first increment.
        buttons = []
        for row in probe.command('ui tree'):
            match = re.search(r'@BUTTON (-?\d+) (-?\d+) (-?\d+) (-?\d+) disabled=\d (.*)', row)
            if match:
                x1, y1, x2, y2 = map(int, match.groups()[:4])
                buttons.append((match[5], (x1 + x2) // 2, (y1 + y2) // 2))
        up = next((x, y) for caption, x, y in buttons if '\uf077' in caption)
        down = next((x, y) for caption, x, y in buttons if '\uf078' in caption)
        assert count(probe) == 1
        for i in range(10):
            touch(probe, up, .1)
            assert count(probe) == i + 2, f'Increment tap {i + 1} was skipped or duplicated'
        for i in range(10):
            touch(probe, down, .1)
            assert count(probe) == 10 - i, f'Decrement tap {i + 1} was skipped or duplicated'
        print('PASS real display: ten consecutive touch taps in each direction', flush=True)
        touch(probe, up, 1.55)
        assert count(probe) == 6, 'Increment hold did not add exactly five'
        touch(probe, down, 1.55)
        assert count(probe) == 1, 'Decrement hold did not subtract exactly five'
        time.sleep(.7)
        assert count(probe) == 1, 'Repeats continued after release'
        print('PASS real display: both holds repeat every 300 ms and stop on release', flush=True)
        touch(probe, up, .1)
        assert count(probe) == 2, 'First tap following a hold was skipped'
        probe.capture('player-selector')
        assert preferences(probe) == original
        after = next(row.split('@STATS ', 1)[1] for row in probe.command('ui stats-data')
                     if '@STATS ' in row)
        assert after == statistics, 'User statistics changed'
    finally:
        try:
            probe.command('ui touch 0 0 0')
            probe.command('ui touch off')
            probe.command('ui home')
            probe.command('ui quiet off')
            probe.command('ui wifi-log on')
        finally:
            probe.close('session')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', default='COM3')
    parser.add_argument('--native-only', action='store_true', help='Skip the live display preview')
    parser.add_argument('--output', type=Path, default=Path('.pio/players-check'))
    run(parser.parse_args())
