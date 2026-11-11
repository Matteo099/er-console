"""Exercise the network picker and password editor with radio-free preview data.

Requires UI_DIAGNOSTICS_ENABLED=1. Uses actual pointer events. Preview handlers
never save credentials or associate Wi-Fi. Original preferences are retained.
"""
import argparse
from pathlib import Path
import re

from ui_settings import preferences, labels
from ui_stability import StabilityProbe


def point(probe, x, y):
    probe.command(f'ui touch {x} {y} 1')
    probe.command('ui touch 0 0 0')
    probe.command('ui touch off')


def tap(probe, caption):
    choices = probe.buttons()
    target = next((button for button in choices if caption in button[0]), None)
    assert target, (caption, choices)
    point(probe,target[1],target[2])


def draft(probe):
    for row in probe.command('ui wifi-draft'):
        match = re.search(r'@WIFI-DRAFT ssid=(.*?) secure=(\d) password_bytes=(\d+)',row)
        if match:
            return match[1], int(match[2]), int(match[3])
    raise AssertionError('No preview draft')


def entry(probe):
    result = probe.command('ui tree')
    assert not any('demo12345' in row or 'qqqqqqqq' in row for row in result), 'Input leaked to diagnostic tree'
    entries = [match for row in result if (match := re.search(r'@TEXTAREA .* masked=(\d) bytes=(\d+)',row))]
    assert len(entries)==1, result
    return tuple(map(int,entries[0].groups()))


def picker(probe, selected):
    probe.command('ui scroll 100')
    tap(probe,selected)


def run(args):
    probe = StabilityProbe(args.port,args.output)
    original = None
    try:
        probe.command('ui quiet on')
        original = preferences(probe)
        assert any(re.search(r'@SELFTEST passed=\d+ failed=0',row) for row in probe.command('ui selftest'))
        for code in ('it','en'):
            cancel, confirm = ('Annulla','Conferma') if code=='it' else ('Cancel','Confirm')
            enter = 'Tocca per inserire' if code=='it' else 'Tap to enter'
            back = 'Indietro' if code=='it' else 'Back'
            probe.command('ui preview-language '+code)
            probe.command('ui wifi-preview ready')
            picker(probe,'Studio')
            rows = probe.buttons()
            assert [next(name for name in ('Studio','Laboratorio') if name in row[0]) for row in rows if 'dBm' in row[0]][:2] == ['Studio','Laboratorio']
            probe.capture('networks-'+code)
            tap(probe,'Laboratorio')  # Another protected SSID must discard the old password.
            assert entry(probe)==(1,0)
            point(probe,40,145)  # Q on the real LVGL keyboard.
            assert entry(probe)[1] > 0
            tap(probe,'\uf06e')  # Eye: show.
            assert entry(probe)[0] == 0
            probe.capture('password-visible-'+code)  # Only dummy text is used in previews.
            tap(probe,cancel)
            assert draft(probe)==('Laboratorio',1,0)
            tap(probe,enter)
            assert entry(probe)==(1,0)  # Every opening restores masking and discards cancelled text.
            for _ in range(8):
                point(probe,40,145)
            masked, size = entry(probe)
            assert masked==1 and size >= 8
            tap(probe,confirm)
            assert draft(probe)==('Laboratorio',1,size)
            picker(probe,'Laboratorio')
            probe.command('ui scrollat 160 140 65')
            tap(probe,'Guest')
            assert draft(probe)==('Guest',0,0)
            assert ('Rete senza password' if code=='it' else 'Open network') in labels(probe)
            assert not any('@TEXTAREA' in row for row in probe.command('ui tree'))
            print(f'PASS {code}: signal/security list, protected selection, show/hide, cancel/confirm, open network',flush=True)
            for state, text in (
                ('empty','Nessuna rete trovata' if code=='it' else 'No networks found'),
                ('error','Ricerca non riuscita' if code=='it' else 'Network search failed'),
                ('busy','Attendi il termine' if code=='it' else 'Wait for the connection'),
                ('scanning','Ricerca delle reti' if code=='it' else 'Searching for networks'),
                ('off','Wi-Fi disabilitato' if code=='it' else 'Wi-Fi disabled'),
            ):
                probe.command('ui wifi-preview '+state)
                picker(probe,'Studio')
                assert any(text in label for label in labels(probe)), state
                tap(probe,back)
            print(f'PASS {code}: empty/error/busy/scanning/disabled states and Back',flush=True)
        print('PASS network UI regression; no radio activation or credential writes',flush=True)
    finally:
        probe.command('ui touch off')
        probe.command('ui settings')  # Restore the persisted language after previews.
        if original:
            assert preferences(probe)==original
        probe.command('ui home')
        probe.close('session')


if __name__=='__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port',default='COM3')
    parser.add_argument('--output',type=Path,default=Path('.pio/network-ui'))
    run(parser.parse_args())
