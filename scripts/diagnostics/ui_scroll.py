"""Run real LVGL pointer, wheel/trackpad and keyboard regression tests on COM3.

Requires debug firmware with UI_DIAGNOSTICS_ENABLED=1. Uses an isolated display;
does not navigate the user's screen, start games, write preferences or play audio.
"""
import argparse
from pathlib import Path
import re

from ui_stability import StabilityProbe


def run(args):
    probe = StabilityProbe(args.port, args.output)
    try:
        result = probe.command('ui scroll-test')
        for row in result:
            if '@SCROLL' in row:
                print(row, flush=True)
        summary = next((re.search(r'@SCROLLTEST passed=(\d+) failed=(\d+)', row)
                        for row in result if '@SCROLLTEST ' in row), None)
        assert summary and int(summary[1]) > 0 and int(summary[2]) == 0, result
    finally:
        probe.close('session')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', default='COM3')
    parser.add_argument('--output', type=Path, default=Path('.pio/scroll-check'))
    run(parser.parse_args())
