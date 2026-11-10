"""Verify OTA engine, UI and optionally native inactive-slot validation.

Requires debug diagnostics. Default probes never install or reboot firmware.
--native-image writes/verifies that binary in the inactive OTA slot, without
changing boot selection. Preferences, real GitHub credentials and statistics
are preserved; the isolated NVS fixture namespace is ota-check.
"""
import argparse
import hashlib
from pathlib import Path
import re
import time

from ui_stability import StabilityProbe
from ui_settings import preferences, labels, tab


def native(probe,path):
    payload=path.read_bytes()
    checksum=hashlib.sha256(payload).hexdigest()
    command=f'ui ota-native {len(payload)} {checksum}'
    probe.port.write((command+'\n').encode())
    probe.read_until(lambda row:'@OTANATIVE READY' in row,timeout=15)
    probe.port.baudrate=921600
    offset=0
    try:
        deadline=time.monotonic()+180
        result=None
        while time.monotonic()<deadline:
            rows=probe.read_until(lambda row:row.startswith('@OTARX ') or row.startswith('@OTANATIVE result='),timeout=30)
            row=rows[-1]
            if row.startswith('@OTANATIVE result='):
                result=row; break
            count=int(row.split()[1]); assert 0<count<=4096 and offset+count<=len(payload)
            probe.port.write(payload[offset:offset+count]); offset+=count
        assert result and 'result=0 unchanged=1' in result,(offset,result)
        assert offset==len(payload),(offset,len(payload))
        print('PASS native esp_ota_begin/write/end and SHA-256 on inactive slot; boot selection unchanged',flush=True)
    finally:
        probe.port.baudrate=115200
    probe.read_until(lambda row:'@UI DONE '+command in row,timeout=10)


def run(args):
    probe=StabilityProbe(args.port,args.output)
    original=None; stats=None; token=None
    try:
        original=preferences(probe)
        stats=next(row.split('@STATS ',1)[1] for row in probe.command('ui stats-data') if '@STATS ' in row)
        token=next(re.search(r'token=(\d)',row)[1] for row in probe.command('ui ota-status') if '@OTA ' in row)
        probe.command('ui audio-master 0'); probe.command('ui home')
        rows=probe.command('ui ota-test')
        result=next((row for row in rows if '@OTATEST' in row),'')
        assert re.search(r'passed=\d+ failed=0',result),rows
        print(result,flush=True)
        probe.command('ui ota-preview'); probe.capture('available')
        install='Installa' if original[1].startswith('it') else 'Install'
        cancel='Annulla' if original[1].startswith('it') else 'Cancel'
        back='Indietro' if original[1].startswith('it') else 'Back'
        probe.click(install); probe.capture('confirmation'); probe.click(cancel)
        probe.click(install); probe.click(install); probe.capture('downloading')
        assert not any(back in title for title,_,_ in probe.buttons()),'Back must be disabled while writing'
        probe.click(cancel); probe.capture('cancelled')
        probe.command('ui ota-preview error'); probe.capture('integrity-error')
        probe.command('ui ota-preview installing'); probe.capture('activating')
        assert not probe.buttons(),'Activation must disable all navigation'
        probe.command('ui settings'); tab(probe,3)
        probe.command('ui scrollat 160 140 180')
        probe.click('Aggiornamenti' if original[1].startswith('it') else 'Updates')
        probe.capture('settings-updates')
        probe.click('Verifica' if original[1].startswith('it') else 'Check')
        # A connected console may need SNTP/TLS; do not change its credentials.
        deadline=time.monotonic()+40
        while time.monotonic()<deadline:
            rows=probe.command('ui ota-status')
            state=int(next(re.search(r'state=(\d+)',row)[1] for row in rows if '@OTA ' in row))
            if state!=1: break
            time.sleep(1)
        assert state!=1,'Update check did not finish within its bounded request window'
        probe.capture('check-result')
        probe.click(back)
        assert any('Risorse' in label or 'Resources' in label for label in labels(probe)), 'Back must retain System tab'
        probe.command('ui home')
        if args.native_image: native(probe,args.native_image)
        assert preferences(probe)==original
        current=next(row.split('@STATS ',1)[1] for row in probe.command('ui stats-data') if '@STATS ' in row)
        assert current==stats,'User game statistics changed'
        current=next(re.search(r'token=(\d)',row)[1] for row in probe.command('ui ota-status') if '@OTA ' in row)
        assert current==token,'User GitHub credential configuration changed'
        print('PASS OTA views, confirmation/cancellation, real bounded check and Settings return; user data preserved',flush=True)
    finally:
        try:
            probe.command('ui home'); probe.command('ui audio-master 100')
        finally:
            probe.close('session')


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port',default='COM3')
    parser.add_argument('--output',type=Path,default=Path('.pio/ota-check'))
    parser.add_argument('--native-image',type=Path)
    run(parser.parse_args())
