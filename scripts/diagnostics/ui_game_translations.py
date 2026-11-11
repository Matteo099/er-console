"""Check PC game assets on the ESP32, without writing SD or operating game hardware.

Uses the debug fixture commands for the real metadata parser, catalog View and
Lua interpreter. Only the object demo's init/update run, with mocked inputs.
"""
import argparse
import json
from pathlib import Path

from ui_settings import labels, preferences
from ui_stability import StabilityProbe


def fast_command(probe, value):
    probe.port.write((value+'\n').encode())
    result = probe.read_until(lambda row: '@UI DONE '+value in row)
    assert not any('@GAMECHECK FAIL' in row for row in result), result
    return result


def fixture(probe, path):
    fast_command(probe, 'ui fixture clear')
    source = path.read_bytes()
    for offset in range(0, len(source), 32):
        fast_command(probe, 'ui fixture append '+source[offset:offset+32].hex())


def run(args):
    root = Path(__file__).resolve().parents[2] / 'sd/games'
    probe = StabilityProbe(args.port,args.output)
    original = None
    try:
        probe.command('ui quiet on')
        original = preferences(probe)
        for config in sorted(root.rglob('config.json')):
            data = json.loads(config.read_text(encoding='utf-8'))
            fixture(probe,config)
            for code in ('it','en'):
                probe.command('ui fixture show '+code)
                assert data['translations'][code]['name'] in labels(probe), (config,code)
                if config.parent.name == '3-t_loop':
                    probe.capture('catalog-'+code)
            fixture(probe,config.with_name('game.lua'))
            mode = 'objects' if config.parent.name == '5-t_gameobject' else 'lua'
            for code in ('it','en'):
                result = fast_command(probe,f'ui fixture {mode} {code}')
                assert any('@GAMECHECK PASS lua' in row for row in result), result
                print(next(row for row in result if '@GAMECHECK PASS lua' in row),flush=True)
            print(f'PASS {config.parent.name}: catalog and Lua in IT/EN',flush=True)
        print('PASS all five bilingual games; object demo also switches language while paused',flush=True)
    finally:
        probe.command('ui fixture clear')
        probe.command('ui settings')  # Reload the persisted language after previews.
        if original:
            assert preferences(probe) == original
        probe.command('ui home')
        probe.close('session')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port',default='COM3')
    parser.add_argument('--output',type=Path,default=Path('.pio/game-translations'))
    run(parser.parse_args())
