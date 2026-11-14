"""Verify internal-NVS settings and persistence across reset.

Requires a debug build with UI_DIAGNOSTICS_ENABLED=1. Tests use an isolated NVS
namespace; production settings are never changed. Saved passwords stay on-device.
"""
import argparse
from pathlib import Path
import re
import time

from ui_settings import preferences
from ui_stability import StabilityProbe
from ui_network_connection import status, wait_connected


def run(args):
    probe = StabilityProbe(args.port, args.output)
    try:
        probe.command('ui wifi-log off')
        original = preferences(probe)
        statistics = next(row.split('@STATS ', 1)[1] for row in probe.command('ui stats-data')
                          if '@STATS ' in row)
        configured = status(probe)[1]
        rows = probe.command('ui config-test')
        for row in rows:
            if '@CONFIG' in row:
                print(row, flush=True)
        result = next((re.search(r'@CONFIGTEST passed=(\d+) failed=(\d+)', row)
                       for row in rows if '@CONFIGTEST ' in row), None)
        assert result and int(result[1]) > 0 and int(result[2]) == 0, rows
        probe.port.setRTS(True)
        time.sleep(.1)
        probe.port.setRTS(False)
        probe.read_until(lambda row: 'Entering MainMenuScene' in row, timeout=30)
        probe.command('ui wifi-log off')
        assert preferences(probe) == original
        assert status(probe)[1] == configured
        if configured:
            wait_connected(probe)
        after = next(row.split('@STATS ', 1)[1] for row in probe.command('ui stats-data')
                     if '@STATS ' in row)
        assert after == statistics, 'User statistics changed'
        print('PASS reboot preserves UI/audio preferences, saved network and statistics', flush=True)
    finally:
        try:
            probe.command('ui wifi-log on')
        finally:
            probe.close('session')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', default='COM3')
    parser.add_argument('--output', type=Path, default=Path('.pio/config-storage-check'))
    run(parser.parse_args())
