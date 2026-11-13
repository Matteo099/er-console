"""Verify internal-NVS settings, safe SD migration and persistence across reset.

Requires a debug build with UI_DIAGNOSTICS_ENABLED=1. Migration cases use RAM
files and isolated NVS namespaces; production settings are never changed.
The reboot comparison retains its baseline in internal NVS and returns boolean
results only: no password, credential hash or configuration export crosses UART.
"""
import argparse
from pathlib import Path
import re
import time

from ui_settings import preferences
from ui_stability import StabilityProbe


def run(args):
    probe = StabilityProbe(args.port, args.output)
    baseline = False
    try:
        probe.command('ui wifi-log off')
        original = preferences(probe)
        statistics = next(row.split('@STATS ', 1)[1] for row in probe.command('ui stats-data')
                          if '@STATS ' in row)
        assert any('@CONFIGBASELINE OK' in row for row in probe.command('ui config-baseline'))
        baseline = True
        rows = probe.command('ui config-test')
        for row in rows:
            if '@CONFIG' in row:
                print(row, flush=True)
        result = next((re.search(r'@CONFIGTEST passed=(\d+) failed=(\d+)', row)
                       for row in rows if '@CONFIGTEST ' in row), None)
        assert result and int(result[1]) > 0 and int(result[2]) == 0, rows
        # The committed baseline stays on the ESP32 across the reset.
        probe.port.setRTS(True)
        time.sleep(.1)
        probe.port.setRTS(False)
        probe.read_until(lambda row: 'Entering MainMenuScene' in row, timeout=30)
        probe.command('ui wifi-log off')
        verification = probe.command('ui config-verify')
        assert any('@CONFIGVERIFY match=1 clean_sd=1 baseline_cleared=1' in row
                   for row in verification), verification
        baseline = False
        assert preferences(probe) == original
        after = next(row.split('@STATS ', 1)[1] for row in probe.command('ui stats-data')
                     if '@STATS ' in row)
        assert after == statistics, 'User statistics changed'
        print('PASS all device settings and credentials survive reboot; no legacy settings on SD',
              flush=True)
        print('PASS UI/audio preferences and statistics preserved', flush=True)
    finally:
        try:
            if baseline:
                probe.command('ui config-verify')
            probe.command('ui wifi-log on')
        finally:
            probe.close('session')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', default='COM3')
    parser.add_argument('--output', type=Path, default=Path('.pio/config-storage-check'))
    run(parser.parse_args())
