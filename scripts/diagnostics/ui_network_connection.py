"""Test real Wi-Fi submission and nonblocking startup on diagnostic firmware.

Temporarily replaces only the saved network with empty/unreachable fixtures.
An exact credential-record backup stays in device NVS across resets and is
restored in finally. No passwords are transferred to the PC. Recovery after
an interrupted test: ui wifi-fixture restore, then reset the console.
"""
import argparse
from pathlib import Path
import re
import time

from ui_stability import StabilityProbe
from ui_settings import preferences, labels


def status(probe):
    rows=probe.command('ui wifi-status')
    value=next((re.search(r'@WIFI state=(\d+) configured=(\d+) timeout=(\d+) ip=(\d+) sdk=(\d+)',row) for row in rows if '@WIFI state=' in row),None)
    assert value, rows
    return tuple(map(int,value.groups()))


def reboot(probe):
    probe.port.reset_input_buffer()
    probe.port.setRTS(True); time.sleep(0.12); probe.port.setRTS(False)
    rows=probe.read_until(lambda row:'Entering MainMenuScene' in row,timeout=20)
    timing=next((re.search(r'\[(\d+)\].*Boot process complete',row) for row in rows if 'Boot process complete' in row),None)
    assert timing and int(timing[1])<10000, 'Boot waited for Wi-Fi association'
    print(f'PASS boot completes in {timing[1]} ms without waiting for association',flush=True)
    probe.command('ui wifi-log off')
    probe.command('ui quiet on')


def fixture(probe, mode):
    assert any('@WIFIFIXTURE OK' in row for row in probe.command('ui wifi-fixture '+mode))


def wait_state(probe, expected, timeout=20):
    deadline=time.monotonic()+timeout
    while time.monotonic()<deadline:
        value=status(probe)
        if value[0]==expected: return value
        time.sleep(0.25)
    raise AssertionError(('Network state did not settle',expected,value))


def wait_connected(probe):
    stable=None; deadline=time.monotonic()+25
    while time.monotonic()<deadline:
        value=status(probe)
        if value[0]==2 and value[3:]==(1,3):
            stable=stable or time.monotonic()
            if time.monotonic()-stable>=2: return
        else:
            stable=None
        time.sleep(0.25)
    raise AssertionError(('Association did not remain connected with an IP',value))


def password_dialog(probe):
    probe.command('ui network')
    for step in range(8):
        target=next((button for button in probe.buttons() if button[0].strip()=='********'),None)
        if target:
            probe.click('********'); return
        probe.command(f'ui scroll {(step+1)*50}')
    raise AssertionError('Saved password button is not reachable')


def submit(probe, confirm, keyboard=False):
    password_dialog(probe)
    if keyboard:
        probe.command('ui touch 294 225 1')
        probe.command('ui touch 0 0 0')
        probe.command('ui touch off')
    else:
        probe.click(confirm)
    value=status(probe)
    assert value[0] in (1,2),('Confirmation did not start association',value)
    assert not any('@TEXTAREA' in row for row in probe.command('ui tree'))
    shown=labels(probe)
    assert any('Connessione in corso' in x or 'Connecting' in x or 'Connessa' in x or 'Connected' in x for x in shown),shown
    print('PASS password confirmation alone starts connection and brings status into view',flush=True)


def capture(probe, name):
    # A serial framebuffer capture is optional; the connection and live-label
    # assertions still run if an unrelated log interrupts the frame.
    try:
        probe.capture(name)
    except ValueError:
        print(f'NOTE {name}: SDK radio logs interrupted the optional screenshot; checking live UI labels instead',flush=True)


def run(args):
    probe=StabilityProbe(args.port,args.output)
    modified=False
    original=preferences(probe)
    configured=status(probe)[1]
    confirm='Conferma' if original[1].startswith('it') else 'Confirm'
    cancel='Annulla' if original[1].startswith('it') else 'Cancel'
    try:
        probe.command('ui wifi-log off')
        probe.command('ui quiet on')
        modified=True
        fixture(probe,'empty'); reboot(probe)
        assert status(probe)[:4]==(0,0,0,0)
        time.sleep(1)
        assert status(probe)[:4]==(0,0,0,0)
        print('PASS no saved credentials: no association request and no false error',flush=True)

        fixture(probe,'unreachable'); reboot(probe)
        assert status(probe)[:4]==(1,1,0,0)
        began=time.monotonic(); probe.command('ui about')
        assert time.monotonic()-began<3,'UI blocked while Wi-Fi connects'
        value=wait_state(probe,4)
        assert value[2:4]==(1,0),value
        probe.command('ui network'); capture(probe,'startup-unreachable')
        assert any('Connessione scaduta' in x or 'Connection timed out' in x for x in labels(probe))
        print('PASS unreachable saved network: startup/UI continue and timeout is displayed',flush=True)

        password_dialog(probe); probe.click(cancel)
        assert status(probe)[0]==4,'Cancel unexpectedly restarted association'
        print('PASS password cancellation does not reconnect',flush=True)
        submit(probe,confirm); capture(probe,'password-connecting')
        assert wait_state(probe,4)[2]==1
        capture(probe,'password-failed')
        assert any('Connessione scaduta' in x or 'Connection timed out' in x for x in labels(probe))
        print('PASS submitted credentials: progress then failure, without a second Save click',flush=True)
    finally:
        try:
            if modified:
                fixture(probe,'restore'); reboot(probe)
                assert status(probe)[1]==configured,'Saved network presence changed'
            assert preferences(probe)==original,'UI/audio preferences changed'
            probe.command('ui home')
            probe.command('ui quiet off')
            probe.command('ui wifi-log on')
        finally:
            probe.close('session')
    if configured:
        # Verify real success separately; restored credentials remain untouched.
        probe=StabilityProbe(args.port,args.output)
        try:
            probe.command('ui wifi-log off')
            wait_connected(probe)
            print('PASS real saved network: automatic boot connection obtains an IP',flush=True)
            submit(probe,confirm)
            wait_connected(probe)
            capture(probe,'password-connected')
            assert any(x.strip() in ('Connessa','Connected') for x in labels(probe))
            print('PASS real network: password submission reconnects and displays success',flush=True)
            submit(probe,confirm,keyboard=True)
            wait_connected(probe)
            print('PASS real keyboard confirmation also reconnects successfully',flush=True)
            probe.command('ui home')
        finally:
            probe.command('ui wifi-log on')
            probe.close('connected-session')
    else:
        print('NOT TESTED successful router association: no user network is configured',flush=True)


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port',default='COM3')
    parser.add_argument('--output',type=Path,default=Path('.pio/wifi-connection-check'))
    run(parser.parse_args())
