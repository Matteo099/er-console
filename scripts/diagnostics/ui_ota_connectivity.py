"""Check OTA over saved Wi-Fi, after Save and Connect, and while disconnected.

Requires diagnostic firmware and an available saved Wi-Fi network. Uses the UI
without reading credentials or changing their values. Never installs firmware.
Preferences/statistics are checked; Wi-Fi is reconnected before returning.
"""
import argparse
from pathlib import Path
import re
import time

from ui_stability import StabilityProbe
from ui_settings import preferences, labels
from ui_network_connection import status, wait_connected


NEEDS_WIFI = (
    'Connetti la console a Internet via Wi-Fi.',
    'Connect the console to Internet over Wi-Fi.',
)


def network_action(probe, caption):
    probe.command('ui network')
    for offset in range(0, 801, 80):
        if any(caption in title for title, _, _ in probe.buttons()):
            probe.click(caption)
            return
        probe.command(f'ui scroll {offset + 80}')
    raise AssertionError('Network action is not reachable: ' + caption)


def check(probe, caption):
    probe.command('ui updates')
    probe.click(caption)
    deadline = time.monotonic() + 50
    while time.monotonic() < deadline:
        rows = probe.command('ui ota-status')
        value = next(re.search(r'@OTA state=(\d+) error=(\d+)', row)
                     for row in rows if '@OTA state=' in row)
        result = tuple(map(int, value.groups()))
        if result[0] != 1:
            return result, labels(probe)
        time.sleep(.5)
    raise AssertionError('OTA check exceeded its bounded request window')


def statistics(probe):
    return next(row.split('@STATS ', 1)[1]
                for row in probe.command('ui stats-data') if '@STATS ' in row)


def run(args):
    probe = StabilityProbe(args.port, args.output)
    disconnected = False
    original = None
    try:
        probe.command('ui wifi-log off')
        probe.command('ui audio-master 0')
        probe.command('ui home')
        original = preferences(probe)
        before = statistics(probe)
        italian = original[1].startswith('it')
        save = 'Salva e connetti' if italian else 'Save and connect'
        disconnect = 'Disconnetti' if italian else 'Disconnect'
        caption = 'Verifica' if italian else 'Check'
        wait_connected(probe)

        # Existing connection must be sufficient; no portal or re-association required.
        result, shown = check(probe, caption)
        assert status(probe)[3:] == (1, 3), 'Saved Wi-Fi lost its IP'
        assert not any(text in shown for text in NEEDS_WIFI), (result, shown)
        assert result[0] in (2, 3) and result[1] == 0, (result, shown)
        print('PASS existing Wi-Fi connection retrieves the latest release', flush=True)

        disconnected = True  # Reconnect in finally even if a later assertion fails.
        network_action(probe, disconnect)
        deadline = time.monotonic() + 15
        while time.monotonic() < deadline:
            value = status(probe)
            if value[0] == 3 and value[4] != 3:
                break
            time.sleep(.2)
        else:
            raise AssertionError(('Wi-Fi did not disconnect', value))
        result, shown = check(probe, caption)
        assert result[0] == 8 and any(text in shown for text in NEEDS_WIFI), (result, shown)
        probe.capture('disconnected')
        print('PASS disconnected Wi-Fi is rejected with connection guidance', flush=True)

        network_action(probe, save)
        wait_connected(probe)
        disconnected = False
        result, shown = check(probe, caption)
        assert status(probe)[3:] == (1, 3)
        assert result[0] in (2, 3) and result[1] == 0, (result, shown)
        assert not any(text in shown for text in NEEDS_WIFI), (result, shown)
        probe.capture('reconnected')
        assert preferences(probe) == original
        assert statistics(probe) == before, 'User statistics changed'
        print('PASS Save and Connect restores OTA checks; user data preserved', flush=True)
    finally:
        try:
            if disconnected and original:
                network_action(probe, 'Salva e connetti' if original[1].startswith('it')
                               else 'Save and connect')
                wait_connected(probe)
            probe.command('ui home')
            probe.command('ui audio-master 100')
            probe.command('ui wifi-log on')
        finally:
            probe.close('session')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', default='COM3')
    parser.add_argument('--output', type=Path, default=Path('.pio/ota-connectivity'))
    run(parser.parse_args())
