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
    probe = StabilityProbe(args.port, args.output)
    started = time.monotonic()
    try:
        time.sleep(2.5)  # Allow an upload-triggered boot to finish before navigating.
        probe.command('ui monitor')  # Registers hooks on both cores, as opening Risorse does.
        probe.click('Indietro')
        original_theme = next(button[0].split()[-1] for button in probe.buttons() if '\uf00c' in button[0])
        probe.click('Sistema')
        original_sound = next(button[0] for button in probe.buttons() if 'Suoni' in button[0])
        probe.click('Indietro')
        for cycle in range(args.cycles):
            probe.click('Info')
            probe.click('Indietro')
            probe.click('Impostazioni')
            probe.click(('Menta', 'Cielo', 'Corallo')[cycle % 3])
            time.sleep(.5)  # Past the 400 ms NVS debounce.
            probe.click('Sistema')
            probe.click('Suoni')
            time.sleep(.5)
            probe.click('Suoni')
            time.sleep(.5)
            probe.click('Risorse')
            probe.command('ui scroll 160')
            probe.command('ui scroll 0')
            probe.click('Indietro')
            probe.click('Indietro')
            print(f'PASS cycle {cycle + 1}/{args.cycles}', flush=True)
        probe.click('Impostazioni')
        probe.click(original_theme)
        time.sleep(.5)
        probe.click('Sistema')
        assert next(button[0] for button in probe.buttons() if 'Suoni' in button[0]) == original_sound
        probe.click('Indietro')
        # Observe a quiet interval too; don't stop reading immediately after the last event.
        for _ in range(10):
            probe.command('ui tree')
            time.sleep(.2)
        commits = sum('UI preferences saved:' in line for line in probe.log)
        sd_errors = sum('Card Failed' in line for line in probe.log)
        assert commits >= args.cycles * 3, f'Too few persisted changes: {commits}'
        print(f'PASS {args.cycles} cycles, {commits} saves, {time.monotonic() - started:.1f}s without reset; SD command failures={sd_errors}', flush=True)
    finally:
        probe.close('session')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', default='COM3')
    parser.add_argument('--cycles', type=int, default=20)
    parser.add_argument('--output', type=Path, default=Path('.pio/ui-stability'))
    options = parser.parse_args()
    if options.cycles < 1:
        parser.error('--cycles must be positive')
    run(options)
