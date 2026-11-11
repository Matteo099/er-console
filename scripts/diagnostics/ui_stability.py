"""Exercise navigation and NVS preferences after enabling the CPU monitor.

Requires the debug firmware with UI_DIAGNOSTICS_ENABLED=1 and a free serial
port. It sends real button events through the existing View/Scene handlers;
it does not start games or execute Lua. Original theme/sound are restored on
success. A failing run retains its serial log and may leave changed preferences.
"""
import argparse
from pathlib import Path
import re
import sys
import time

sys.dont_write_bytecode = True
from ui_capture import Probe


class StabilityProbe(Probe):
    def read_until(self, predicate, timeout=30):
        def checked(line):
            if re.search(r'Guru Meditation|assert failed|Rebooting|Brownout|watchdog got triggered', line, re.I):
                if 'Rebooting' not in line:
                    try:
                        super(StabilityProbe, self).read_until(lambda row: 'Rebooting' in row, timeout=3)
                    except TimeoutError:
                        pass
                raise RuntimeError('Device failure: ' + line)
            return predicate(line)
        return super().read_until(checked, timeout)

    def buttons(self):
        for _ in range(10):
            result = []
            moving = False
            for line in self.command('ui tree'):
                match = re.search(r'@BUTTON (-?\d+) (-?\d+) (-?\d+) (-?\d+) disabled=(\d) (.*)', line)
                if match:
                    x1, y1, x2, y2, disabled = map(int, match.groups()[:5])
                    moving |= x1 < 0 or x2 > 320
                    moving |= not match[6].strip()  # Incomplete serial row during verbose Lua logs.
                    if not disabled and 0 <= (y1 + y2) // 2 < 240:
                        result.append((match[6], (x1 + x2) // 2, (y1 + y2) // 2))
            if not moving:
                return result
        raise AssertionError('Screen transition did not settle')

    def click(self, title):
        # onEnter can block on SD before LVGL activates the incoming screen.
        # Wait for its controls, then use settled coordinates rather than stale ones.
        for _ in range(6):
            choices = self.buttons()
            target = next((button for button in choices if title in button[0]), None)
            if target:
                self.command(f'ui click {target[1]} {target[2]}')
                return
        raise AssertionError(f'Button {title!r} not available: {choices}')


def run(args):
    # Keep the established command while sharing the current Settings regression.
    from ui_settings import run as settings_run
    args.reset = False
    settings_run(args)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', default='COM3')
    parser.add_argument('--cycles', type=int, default=20)
    parser.add_argument('--output', type=Path, default=Path('.pio/ui-stability'))
    options = parser.parse_args()
    if options.cycles < 1:
        parser.error('--cycles must be positive')
    run(options)
