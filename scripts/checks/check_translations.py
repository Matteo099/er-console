"""Validate the firmware's bilingual catalog and printf argument signatures."""
import json
from pathlib import Path
import re

catalog = Path(__file__).resolve().parents[2] / 'src/systems/ui/localization/Translations.def'
row = re.compile(r'X\((\w+),\s*("(?:[^"\\]|\\.)*"),\s*("(?:[^"\\]|\\.)*")\)')
printf = re.compile(r'%(?:[-+ #0]*\d*(?:\.\d+)?(?:hh|ll|[hljztL])?[diuoxXfFeEgGaAcsp]|%)')
keys = set()
errors = []
for number, line in enumerate(catalog.read_text(encoding='utf-8').splitlines(), 1):
    if not line.strip() or line.lstrip().startswith('//'):
        continue
    match = row.fullmatch(line.strip())
    if not match:
        errors.append(f'{number}: invalid catalog row')
        continue
    key, italian, english = match.groups()
    italian, english = json.loads(italian), json.loads(english)
    if key in keys:
        errors.append(f'{number}: duplicate key {key}')
    keys.add(key)
    if key != 'None' and (not italian.strip() or not english.strip()):
        errors.append(f'{number}: incomplete translation {key}')
    if printf.findall(italian) != printf.findall(english):
        errors.append(f'{number}: mismatched printf signature {key}')
    # The shared reading fonts cover ASCII/Latin-1 and the password bullet.
    # Other characters require extending the font assets before adding UI text.
    unsupported = {char for char in italian + english if ord(char) > 255 and char != '\u2022'}
    if unsupported:
        errors.append(f'{number}: unsupported reading-font characters {key}: {unsupported}')
if errors:
    raise SystemExit('\n'.join(errors))
print(f'PASS {len(keys)} translation keys, two languages, matching printf signatures and reading-font coverage')
