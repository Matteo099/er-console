"""Verify bounded game-source prefetch, real PNG decoding and a live game.

Requires debug UI diagnostics and the supplied SD assets. No test files are
written to SD; real preferences/statistics are preserved by the game probe.
"""
import argparse
from pathlib import Path
import re
from ui_stability import StabilityProbe
from audio_preload_game import run as game_run


def run(args):
    probe=StabilityProbe(args.port,args.output/'cache')
    try:
        probe.command('ui home')
        rows=probe.command('ui assets-test')
        result=next((row for row in rows if '@ASSETTEST' in row),'')
        assert re.search(r'@ASSETTEST passed=\d+ failed=0',result),rows
        print(result,flush=True)
    finally:
        probe.close('session')
    game_run(argparse.Namespace(port=args.port,output=args.output/'game'))


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port',default='COM3')
    parser.add_argument('--output',type=Path,default=Path('.pio/game-assets-check'))
    run(parser.parse_args())
