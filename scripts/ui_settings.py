"""Exercise Settings through actual LVGL events; restore original preferences.

Requires debug firmware with UI_DIAGNOSTICS_ENABLED=1 and a free COM3.
No game, Wi-Fi connection, credential update or audible playback is started.
"""
import argparse
import json
from pathlib import Path
import re
import time

from ui_stability import StabilityProbe
from ui_capture import Probe


def preferences(probe):
    for row in probe.command('ui preferences'):
        match = re.search(r'@PREFERENCES theme=(\w+) language=([\w-]+) music=(\d)/(\d+) effects=(\d)/(\d+)', row)
        if match:
            return match[1], match[2], tuple(map(int, match.groups()[2:]))
    raise AssertionError('No preferences response')


def labels(probe):
    return [match[1] for row in probe.command('ui tree')
            if (match := re.search(r'@LABEL -?\d+ -?\d+ -?\d+ -?\d+ (.*)', row))]

def confirm_players(probe):
    captions=labels(probe)
    if any('Quanti giocatori' in text or 'How many players' in text for text in captions):
        probe.click('Avvia' if any('Quanti giocatori' in text for text in captions) else 'Start')

def begin_statistics_sandbox(probe):
    original=next(json.loads(row.split('@STATS ',1)[1]) for row in probe.command('ui stats-data') if '@STATS ' in row)
    assert any('@STATSSANDBOX OK' in row for row in probe.command('ui stats-sandbox on'))
    return original

def end_statistics_sandbox(probe,original):
    assert any('@STATSSANDBOX OK' in row for row in probe.command('ui stats-sandbox off'))
    restored=next(json.loads(row.split('@STATS ',1)[1]) for row in probe.command('ui stats-data') if '@STATS ' in row)
    assert restored==original,'User statistics changed during sandbox test'


def tab(probe, index):
    probe.command(f'ui click {45 + index * 76} 70')


def audio(probe, values, drag=False):
    tab(probe, 1)
    for index, (enabled, volume) in enumerate(zip(values[::2], values[1::2])):
        probe.command(f'ui scrollat 160 140 {index * 96}')
        controls = {}
        for row in probe.command('ui tree'):
            match = re.search(r'@(SWITCH|SLIDER) (-?\d+) (-?\d+) (-?\d+) (-?\d+) value=(\d+)', row)
            if match:
                x1, y1, x2, y2 = map(int, match.groups()[1:5])
                if 100 <= (y1 + y2) // 2 < 186:
                    controls[match[1]] = (x1, (y1+y2)//2, x2, int(match[6]))
        assert set(controls) == {'SWITCH', 'SLIDER'}, controls
        for kind, value in [('SWITCH', enabled), ('SLIDER', volume)]:
            x1, y, x2, current = controls[kind]
            if kind == 'SLIDER' and drag:
                start = round(x1 + (x2-x1) * current / 100)
                finish = round(x1 + (x2-x1) * value / 100)
                probe.command(f'ui touch {start} {y} 1')
                for step in range(1, 7):
                    x = round(start + (finish-start) * step / 6)
                    probe.command(f'ui touch {x} {y} 1')
                # Emulate the exact resistive-panel release that caused zeroing.
                probe.command('ui touch 0 0 0')
                probe.command('ui touch off')
            else:
                probe.command(f'ui set {(x1+x2)//2} {y} {value}')
    time.sleep(.5)


def language(probe, code):
    tab(probe, 2)
    probe.click('English' if code.startswith('en') else 'Italiano')
    time.sleep(.5)


def run(args):
    probe = StabilityProbe(args.port, args.output)
    original = None
    try:
        probe.command('ui quiet on')
        original = preferences(probe)
        args.output.joinpath('original-preferences.txt').write_text(repr(original), encoding='utf-8')
        assert any(re.search(r'@SELFTEST passed=\d+ failed=0', row) for row in probe.command('ui selftest'))
        probe.command('ui settings')
        language(probe, 'en-GB')
        audio(probe, (1, 35, 0, 12), drag=True)
        assert preferences(probe)[1:] == ('en-GB', (1, 35, 0, 12))
        audio(probe, (0, 65, 1, 80), drag=True)
        assert preferences(probe)[1:] == ('en-GB', (0, 65, 1, 80))
        print('PASS actual pointer drags, release at (0,0), independent volumes and toggles', flush=True)
        probe.capture('audio-effects-en')
        probe.click('Back')
        assert 'Settings' in labels(probe) and 'Play' in labels(probe)
        if args.reset:
            probe.port.reset_input_buffer()
            probe.port.setRTS(True); time.sleep(.1); probe.port.setRTS(False)
            Probe.read_until(probe, lambda row: 'Entering MainMenuScene' in row, timeout=30)
            time.sleep(.5)
            probe.command('ui quiet on')
            assert preferences(probe)[1:] == ('en-GB', (0, 65, 1, 80))
            assert 'Settings' in labels(probe)
            print('PASS NVS persistence and English boot after hardware reset', flush=True)
        probe.click('Settings')
        language(probe, 'it-IT')
        assert 'Impostazioni' in labels(probe)
        probe.capture('language-it')
        tab(probe, 3)
        for cycle in range(args.cycles):
            probe.click('Risorse')
            probe.command('ui scroll 160'); probe.command('ui scroll 0')
            probe.click('Indietro')
            assert any('Rete e connessioni' in caption for caption, _, _ in probe.buttons())
            probe.click('Rete e connessioni')
            assert 'Connessioni' in labels(probe)
            probe.command('ui scroll 150'); probe.command('ui scroll 0')
            probe.click('Indietro')
            assert any('Risorse' in caption for caption, _, _ in probe.buttons())
            tab(probe, 0)
            probe.click(('Menta', 'Cielo', 'Corallo')[cycle % 3])
            time.sleep(.5)  # Save to NVS while CPU monitor hooks remain registered.
            tab(probe, 3)
            print(f'PASS resources/network return to System {cycle+1}/{args.cycles}', flush=True)
        print('PASS Settings regression', flush=True)
    finally:
        probe.command('ui touch off')
        if original:
            try:
                theme, code, values = original
                probe.command('ui settings')
                audio(probe, values)
                language(probe, code)
                tab(probe, 0)
                name = {'mint': ('Menta', 'Mint'), 'sky': ('Cielo', 'Sky'), 'coral': ('Corallo', 'Coral')}[theme][code.startswith('en')]
                probe.click(name)
                time.sleep(.5)
                assert preferences(probe) == original
                probe.command('ui home')
                print('PASS original preferences restored', flush=True)
            except Exception as error:
                print(f'RESTORE FAILED: {error}; backup: {args.output}/original-preferences.txt', flush=True)
                raise
            finally:
                probe.close('session')
        else:
            probe.close('session')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', default='COM3')
    parser.add_argument('--output', type=Path, default=Path('.pio/settings-regression'))
    parser.add_argument('--cycles', type=int, default=5)
    parser.add_argument('--reset', action='store_true')
    run(parser.parse_args())
