"""Exercise sessions and statistics through the real Loop game in a RAM sandbox.

Does not alter user statistics/preferences. Uses a separate NVS test key for
the persistence self-test and temporarily sets master to 1% for audio.
"""
import argparse
import json
from pathlib import Path
import time

from ui_settings import labels, preferences
from ui_stability import StabilityProbe


def data(probe):
    return next(json.loads(row.split('@STATS ',1)[1]) for row in probe.command('ui stats-data') if '@STATS ' in row)


def loop_game(probe,start):
    probe.command('ui games')
    for _ in range(5):
        if any('Test - Ciclo' in text or 'Test - Loop' in text for text in labels(probe)):
            break
        probe.click('\uf054')
    else:
        raise AssertionError('Loop game not found')
    offset=len(probe.log); probe.click(start)
    if not any('Game loaded: 3-t_loop' in row for row in probe.log[offset:]):
        probe.read_until(lambda row:'Game loaded: 3-t_loop' in row,timeout=35)
    probe.click(start)
    assert any('Quanti giocatori' in text or 'How many players' in text for text in labels(probe))


def wait_outcome(probe,offset,outcome,timeout=5):
    if not any('Session finished:' in row and f'outcome={outcome} ' in row for row in probe.log[offset:]):
        probe.read_until(lambda row:'Session finished:' in row and f'outcome={outcome} ' in row,timeout=timeout)


def run(args):
    probe=StabilityProbe(args.port,args.output); original=None; sandbox=False
    try:
        original=preferences(probe); production=data(probe)
        assert any('@STATSTEST passed=26 failed=0' in row for row in probe.command('ui stats-test'))
        assert any('@STATSSANDBOX OK' in row for row in probe.command('ui stats-sandbox on')); sandbox=True
        probe.command('ui audio-master 1')
        start='Avvia' if original[1].startswith('it') else 'Start'
        cancel='Annulla' if original[1].startswith('it') else 'Cancel'
        back='Indietro' if original[1].startswith('it') else 'Back'
        loop_game(probe,start)
        assert data(probe)['s'][0]==0,'Opening player selection counted a game'
        probe.capture('players-minimum')
        probe.click('\uf077'); probe.click('\uf077'); probe.click('\uf078'); probe.click('\uf077')
        assert '3' in labels(probe)
        probe.click(cancel)
        assert data(probe)['s'][0]==0,'Cancelled selection counted a game'
        probe.click(start); probe.click('\uf077'); probe.click('\uf077'); probe.capture('players-three')
        before=time.monotonic(); probe.click(start)
        active=data(probe)
        assert active['s'][0]==1 and active['a'][2]==3
        time.sleep(1.2)
        probe.click('Pausa' if original[1].startswith('it') else 'Pause'); time.sleep(2.2)
        probe.click(start)
        assert not any('Quanti giocatori' in text or 'How many players' in text for text in labels(probe)),'Resume asked for players again'
        time.sleep(1.1)
        offset=len(probe.log); probe.command('ui stats-win'); wait_outcome(probe,offset,1)
        won=data(probe); first=won['h'][-1]
        assert won['s'][0:4]==[1,1,0,0] and won['s'][4]==3 and first[2:4]==[3,1]
        assert first[4]<(time.monotonic()-before)*1000-1800,'Paused time was counted in duration'
        probe.command('ui scrollat 160 130 180'); probe.capture('victory-players')
        print('PASS 3-player win, cancellation, resume and pause-excluded duration',flush=True)
        probe.command('ui games')
        for _ in range(5):
            if any('Test - Ciclo' in text or 'Test - Loop' in text for text in labels(probe)): break
            probe.click('\uf054')
        probe.click('\uf00b')
        assert any('Test - Ciclo' in text or 'Test - Loop' in text for text in labels(probe))
        probe.capture('per-game-statistics')
        probe.click(back)
        assert any('Test - Ciclo' in text or 'Test - Loop' in text for text in labels(probe)),'Back lost selected game'
        print('PASS per-game statistics and return to selected game',flush=True)
        loop_game(probe,start); offset=len(probe.log); probe.click(start)
        assert data(probe)['a'][2]==1
        print('PASS one-player session started; waiting for natural one-minute loss',flush=True)
        wait_outcome(probe,offset,2,70)
        lost=data(probe); last=lost['h'][-1]
        assert lost['s'][0:4]==[2,1,1,0] and lost['s'][4]==4 and last[2:4]==[1,2] and last[4]==60000
        assert lost['s'][5]==first[4]+60000 and lost['s'][6]==lost['s'][7]==first[4]
        print('PASS natural loss, weighted average and victory-only best/worst',flush=True)
        loop_game(probe,start); probe.click('\uf077'); probe.click(start); time.sleep(.6)
        offset=len(probe.log); probe.click('\uf00d')
        probe.click('Abbandona' if original[1].startswith('it') else 'Leave')
        wait_outcome(probe,offset,3)
        abandoned=data(probe)
        assert abandoned['s'][0:5]==[3,1,1,1,6] and abandoned['s'][5]==lost['s'][5]
        assert abandoned['h'][-1][2:4]==[2,3]
        print('PASS abandonment is separate from a loss and preserves completed averages',flush=True)
        probe.command('ui settings'); probe.command('ui click 273 70')
        probe.command('ui scrollat 160 140 90')
        probe.click('Statistiche' if original[1].startswith('it') else 'Statistics')
        assert ('Tutti i giochi' if original[1].startswith('it') else 'All games') in labels(probe)
        probe.capture('global-statistics')
        probe.click(back)
        assert any('Risorse' in text or 'Resources' in text for text in labels(probe)),'Global stats did not return to System'
        assert preferences(probe)==original
        print('PASS global statistics through Settings and original preferences preserved',flush=True)
    finally:
        try:
            probe.command('ui home')
            if sandbox:
                assert any('@STATSSANDBOX OK' in row for row in probe.command('ui stats-sandbox off'))
                assert data(probe)==production,'User statistics changed during sandbox tests'
            probe.command('ui audio-master 100')
        finally:
            probe.close('session')


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port',default='COM3')
    parser.add_argument('--output',type=Path,default=Path('.pio/statistics-check'))
    run(parser.parse_args())
