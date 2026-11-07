"""Exercise in-game audio settings through the real one-minute Loop game.

Requires debug diagnostics. Loop disables all hardware input subsystems. Tests
use master 1%, save a preference backup and restore the original audio values.
"""
import argparse
from pathlib import Path
import re
import time

from ui_settings import labels, preferences, confirm_players, begin_statistics_sandbox, end_statistics_sandbox, audio as restore_audio
from ui_stability import StabilityProbe


def timer(probe):
    values=[text for text in labels(probe) if re.fullmatch(r'\d{2}:\d{2}:\d{2}',text)]
    assert values,'No game timer visible'
    hours,minutes,seconds=map(int,values[-1].split(':'))
    return hours*3600+minutes*60+seconds


def open_settings(probe):
    start=len(probe.log)
    probe.click('\uf013')
    rows=probe.log[start:]
    assert any('In-game settings opened' in row for row in rows),rows
    return rows


def set_channel(probe,index,enabled,volume,drag=False):
    probe.command(f'ui scrollat 160 120 {index*100}')
    controls={}
    for row in probe.command('ui tree'):
        match=re.search(r'@(SWITCH|SLIDER) (-?\d+) (-?\d+) (-?\d+) (-?\d+) value=(\d+)',row)
        if match:
            x1,y1,x2,y2=map(int,match.groups()[1:5]); y=(y1+y2)//2
            if 48 <= y < 192 and match[1] not in controls:
                controls[match[1]]=(x1,y,x2,int(match[6]))
    assert set(controls)=={'SWITCH','SLIDER'},controls
    x1,y,x2,current=controls['SWITCH']
    probe.command(f'ui set {(x1+x2)//2} {y} {int(enabled)}')
    x1,y,x2,current=controls['SLIDER']
    if drag:
        begin=round(x1+(x2-x1)*current/100); end=round(x1+(x2-x1)*volume/100)
        probe.command(f'ui touch {begin} {y} 1')
        for step in range(1,5):
            probe.command(f'ui touch {round(begin+(end-begin)*step/4)} {y} 1')
        probe.command('ui touch 0 0 0'); probe.command('ui touch off')
    else:
        probe.command(f'ui set {(x1+x2)//2} {y} {volume}')
    time.sleep(.5)


def run(args):
    probe=StabilityProbe(args.port,args.output)
    original=None
    original_stats=None
    try:
        original=preferences(probe)
        original_stats=begin_statistics_sandbox(probe)
        args.output.joinpath('original-preferences.txt').write_text(repr(original),encoding='utf-8')
        rows=probe.command('ui ingame-test')
        assert any(re.search(r'@INGAMETEST passed=\d+ failed=0',row) for row in rows),rows
        print(next(row for row in rows if '@INGAMETEST' in row),flush=True)
        probe.command('ui audio-master 1'); probe.command('ui games')
        for _ in range(5):
            if any('Test - Ciclo' in text or 'Test - Loop' in text for text in labels(probe)):
                break
            probe.click('\uf054')
        else:
            raise AssertionError('Loop game not found')
        caption='Avvia' if original[1].startswith('it') else 'Start'
        entered=len(probe.log); probe.click(caption)
        if not any('Game loaded: 3-t_loop' in row for row in probe.log[entered:]):
            probe.read_until(lambda row:'Game loaded: 3-t_loop' in row,timeout=35)
        probe.click(caption)
        confirm_players(probe)
        live_start=len(probe.log)
        rows=open_settings(probe)
        assert any('state=1 ' in row for row in rows),rows
        before=timer(probe)
        set_channel(probe,0,False,24)
        set_channel(probe,1,True,18,drag=True)
        assert preferences(probe)==(original[0],original[1],(0,24,1,18)),preferences(probe)
        after=timer(probe)
        assert 0 < after < before,(before,after)
        scope=probe.log[live_start:]
        assert any('gioco attivo' in row or 'game active' in row for row in scope),scope
        assert not any('gioco in pausa' in row or 'game paused' in row for row in scope),scope
        probe.capture('running-settings')
        back='Torna al gioco' if original[1].startswith('it') else 'Back to game'
        probe.click(back)
        assert any('In-game settings closed; state=1 ' in row for row in probe.log[live_start:])
        print('PASS live sliders persist immediately while timer and Lua continue without scene reload',flush=True)
        probe.click('Pausa' if original[1].startswith('it') else 'Pause')
        rows=open_settings(probe)
        assert any('state=2 ' in row for row in rows),rows
        paused=timer(probe); time.sleep(2.2)
        assert timer(probe)==paused,'Settings resumed a paused game'
        probe.capture('paused-settings')
        probe.click(back)
        assert timer(probe)==paused,'Closing settings changed the paused timer'
        print('PASS a paused game remains paused across opening and closing settings',flush=True)
        probe.click(caption); open_settings(probe)
        values=original[2]
        set_channel(probe,0,values[0],values[1]); set_channel(probe,1,values[2],values[3])
        assert preferences(probe)==original,'Original preferences were not restored'
        print('PASS original preferences restored; waiting for timeout with settings open',flush=True)
        probe.read_until(lambda row:'Showing fail subview' in row,timeout=70)
        assert back not in labels(probe),'Settings overlay covers the end-of-game screen'
        assert sum('Entering GameScene: 3-t_loop' in row for row in probe.log[entered:])==1,'GameScene was recreated'
        assert sum('Game loaded: 3-t_loop' in row for row in probe.log[entered:])==1,'Lua game was reloaded'
        assert preferences(probe)==original
        print('PASS natural timeout dismisses settings without reloading the game or losing preferences',flush=True)
    finally:
        try:
            probe.command('ui touch off')
            if original and preferences(probe)!=original:
                probe.command('ui settings')
                restore_audio(probe,original[2])
                assert preferences(probe)==original,'Preference restoration failed'
            probe.command('ui home')
            if original_stats is not None: end_statistics_sandbox(probe,original_stats)
            probe.command('ui audio-master 100')
        finally:
            probe.close('session')


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port',default='COM3')
    parser.add_argument('--output',type=Path,default=Path('.pio/ingame-settings-check'))
    run(parser.parse_args())
