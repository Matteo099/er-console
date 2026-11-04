"""Measure scene preparation and LVGL refresh on debug firmware with UI diagnostics.

Requires pyserial and a free serial port. No game scripts are executed.
--quiet temporarily mutes audio without saving preferences. JSON/serial logs
are kept in --output. Serial round trips include settling and are not touch latency.
"""
import argparse
import json
from pathlib import Path
import re
import sys
import time

sys.dont_write_bytecode = True
from ui_stability import StabilityProbe


def run(args):
    probe = StabilityProbe(args.port, args.output)
    results = []
    try:
        probe.port.reset_input_buffer()
        if args.quiet: probe.command('ui quiet on')
        probe.command('ui home')
        time.sleep(1)
        for cycle in range(args.cycles):
            for title in ('Impostazioni', 'Indietro', 'Info', 'Indietro'):
                probe.command('ui perf reset')
                probe.click(title)
                time.sleep(.6)
                row = next(row for row in probe.command('ui perf') if '@PERF' in row)
                metrics = {key: int(value) for key, value in re.findall(r'(\w+)=(\d+)', row)}
                results.append({'cycle': cycle, 'scene': title, **metrics})
                print(title, row, flush=True)
        for cycle in range(2):
            probe.command('ui perf reset')
            probe.command('ui games')
            time.sleep(.6)
            row = next(row for row in probe.command('ui perf') if '@PERF' in row)
            metrics = {key: int(value) for key, value in re.findall(r'(\w+)=(\d+)', row)}
            results.append({'cycle': cycle, 'scene': 'Catalogo', **metrics})
            print('Catalogo', row, flush=True)
            probe.command('ui home')
        (args.output/'results.json').write_text(json.dumps(results, indent=2))
    finally:
        if args.quiet:
            try: probe.command('ui quiet off')
            except (RuntimeError, TimeoutError): pass
        probe.close('session')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', default='COM3')
    parser.add_argument('--cycles', type=int, default=3)
    parser.add_argument('--output', type=Path, default=Path('.pio/ui-performance'))
    parser.add_argument('--quiet', action='store_true')
    options = parser.parse_args()
    if options.cycles < 1: parser.error('--cycles must be positive')
    run(options)
