"""Check bilingual game metadata, message keys, printf arguments and font coverage."""
import json
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1] / 'sd/games'
pair = re.compile(r'^\s*(\w+)\s*=\s*\{it\s*=\s*("(?:[^"\\]|\\.)*"),\s*en\s*=\s*("(?:[^"\\]|\\.)*")\},?$', re.M)
printf = re.compile(r'%(?:[-+ #0]*\d*(?:\.\d+)?(?:hh|ll|[hljztL])?[diuoxXfFeEgGaAcsp]|%)')
count = 0
for path in sorted(root.rglob('config.json')):
    value = json.loads(path.read_text(encoding='utf-8'))
    for code in ('it', 'en'):
        for field in ('name', 'description'):
            assert value['translations'][code][field].strip(), (path, code, field)
    assert value['name'] == value['translations']['it']['name'], path
    assert value['description'] == value['translations']['it']['description'], path
    source = path.with_name('game.lua').read_text(encoding='utf-8')
    messages = {}
    for match in pair.finditer(source):
        key, italian, english = match.groups()
        assert key not in messages, (path, 'duplicate', key)
        italian, english = json.loads(italian), json.loads(english)
        assert italian and english, (path, key)
        assert printf.findall(italian) == printf.findall(english), (path, key, 'printf')
        assert all(ord(char) <= 255 for char in italian+english), (path, key, 'font')
        messages[key] = (italian, english)
    assert messages, (path, 'no Lua messages')
    used = set(re.findall(r'(?<![\w.])text\("(\w+)"\)', source))
    assert used <= messages.keys(), (path, 'unknown keys', used-messages.keys())
    assert 'system.translate(messages[key])' in source, path
    count += len(messages)
    print(f'PASS {path.parent.name}: metadata IT/EN, {len(messages)} Lua messages')
print(f'PASS {count} bilingual game messages; printf signatures and reading-font coverage')
