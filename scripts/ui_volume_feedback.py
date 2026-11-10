"""Check volume feedback scheduling, per-bus PCM gain and slider events on ESP32.

Requires debug firmware with UI_DIAGNOSTICS_ENABLED=1 and a free COM3. Tests
use isolated queues/PCM buffers and do not play sounds or write preferences.
"""
import argparse
from pathlib import Path
import re

from ui_stability import StabilityProbe


def run(args):
    probe = StabilityProbe(args.port,args.output)
    try:
        for command, prefix in [('ui volume-test','VOLUME'),('ui audio-test','AUDIO'),('ui selftest','SELF')]:
            result = probe.command(command)
            summary = next((re.search(rf'@{prefix}TEST passed=(\d+) failed=(\d+)',row)
                            for row in result if f'@{prefix}TEST ' in row),None)
            for row in result:
                if f'@{prefix}' in row:
                    print(row,flush=True)
            assert summary and int(summary[1]) > 0 and int(summary[2]) == 0,result
    finally:
        probe.close('session')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port',default='COM3')
    parser.add_argument('--output',type=Path,default=Path('.pio/volume-feedback-check'))
    run(parser.parse_args())
