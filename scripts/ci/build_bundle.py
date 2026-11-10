"""Build and verify a traceable firmware + portal + complete runtime SD bundle."""
import argparse
import csv
import hashlib
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time
import zipfile

from build_metadata import HARDWARE, metadata


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def tracked_sd(root):
    names = subprocess.check_output(['git', '-C', str(root), 'ls-files', '-z', '--', 'sd'], text=True)
    contents = {}
    for name in names.split('\0'):
        if not name:
            continue
        relative = Path(name).relative_to('sd').as_posix()
        # Runtime settings/credentials and old generated frontend never ship.
        if relative.startswith(('.core/config/', '.core/portal/')):
            continue
        path = root/name
        if path.is_symlink() or not path.resolve().is_relative_to(root.resolve()):
            raise ValueError(f'Unsafe asset path: {name}')
        contents[relative] = path.read_bytes()
    return contents


def zip_contents(path, contents, epoch):
    stamp = time.gmtime(max(epoch, 315532800))[:6]
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
        for name, data in sorted(contents.items()):
            info = zipfile.ZipInfo(name, stamp)
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = 0o100644 << 16
            archive.writestr(info, data)


def slot_size(partitions):
    sizes = []
    for line in partitions.read_text(encoding='utf-8').splitlines():
        if not line.strip() or line.lstrip().startswith('#'):
            continue
        values = [value.strip() for value in next(csv.reader([line]))]
        if len(values)>=5 and values[1]=='app' and values[2] in ('ota_0', 'ota_1'):
            size = values[4].lower()
            sizes.append(int(size[:-1], 0)*1024 if size.endswith('k') else int(size, 0))
    if len(sizes)!=2:
        raise ValueError('Expected two OTA slots in the selected partition table')
    return min(sizes)


def package(root, output, info, firmware, elf, partitions, frontend):
    firmware_bytes = firmware.read_bytes()
    maximum = slot_size(partitions)
    if not firmware_bytes or firmware_bytes[0]!=0xE9 or len(firmware_bytes)>maximum:
        raise ValueError(f'Invalid ESP application or firmware exceeds OTA slot ({maximum} bytes)')
    portal = {path.relative_to(frontend).as_posix(): path.read_bytes()
              for path in sorted(frontend.rglob('*')) if path.is_file() and not path.is_symlink()}
    if not portal.get('index.html') or not any(name.endswith('.gz') for name in portal):
        raise ValueError('Compiled frontend with gzip assets is required')
    info_bytes = (json.dumps(info, indent=2)+'\n').encode()
    portal['build-info.json'] = info_bytes
    sd = tracked_sd(root)
    for required in ('.core/audio/tickingclock.wav', '.core/audio/lose.wav', '.core/audio/win.wav', '.core/images/logo.png'):
        if not sd.get(required):
            raise ValueError(f'Missing runtime SD asset: {required}')
    games = [name for name in sd if name.startswith('games/') and name.endswith('/config.json')]
    if not games or any(not sd.get(name.removesuffix('config.json')+'game.lua') for name in games):
        raise ValueError('Every packaged game must include configuration and a Lua entry point')
    sd.update({'.core/portal/'+name: data for name, data in portal.items()})
    sd['build-info.json'] = info_bytes
    sd['.core/config/README.md'] = b'User configuration and Wi-Fi credentials are intentionally not distributed.\n'
    output.mkdir(parents=True, exist_ok=True)
    stem = f'escaperiom-{HARDWARE}-{info["version"]}'
    binary = output/f'{stem}.bin'
    binary.write_bytes(firmware_bytes)
    portal_zip, sd_zip, symbols_zip = [output/f'{stem}-{suffix}.zip' for suffix in ('portal', 'sd', 'symbols')]
    zip_contents(portal_zip, portal, info['source_date_epoch'])
    zip_contents(sd_zip, sd, info['source_date_epoch'])
    if not elf.read_bytes().startswith(b'\x7fELF'):
        raise ValueError('Matching firmware ELF is required for crash diagnostics')
    zip_contents(symbols_zip, {'firmware.elf': elf.read_bytes(), 'build-info.json': info_bytes}, info['source_date_epoch'])
    files = {}
    for kind, path in [('firmware', binary), ('portal', portal_zip), ('sd', sd_zip), ('symbols', symbols_zip)]:
        data = path.read_bytes()
        files[kind] = {'file': path.name, 'size': len(data), 'sha256': sha256(data)}
    manifest = {
        'schema_version': 1, **info, 'platformio_environment': HARDWARE,
        'firmware_type': 'application', 'ota_slot_size': maximum,
        'partition_table_sha256': sha256(partitions.read_bytes()),
        'games': len(games), 'sd_files': len(sd), 'portal_files': len(portal), 'files': files,
    }
    (output/'manifest.json').write_text(json.dumps(manifest, indent=2)+'\n', encoding='utf-8')
    checksum_paths = [binary, portal_zip, sd_zip, symbols_zip, output/'manifest.json']
    if (output/'CHANGELOG.md').is_file():
        checksum_paths.append(output/'CHANGELOG.md')
    (output/'SHA256SUMS').write_text(''.join(f'{sha256(path.read_bytes())}  {path.name}\n' for path in checksum_paths), encoding='utf-8')
    verify(output)
    return manifest


def verify(output):
    manifest = json.loads((output/'manifest.json').read_text(encoding='utf-8'))
    for record in manifest['files'].values():
        path = output/record['file']
        if path.parent.resolve()!=output.resolve():
            raise ValueError('Artifact filename must not escape the bundle directory')
        data = path.read_bytes()
        if len(data)!=record['size'] or sha256(data)!=record['sha256']:
            raise ValueError(f'Artifact checksum mismatch: {path.name}')
    for line in (output/'SHA256SUMS').read_text(encoding='utf-8').splitlines():
        expected, name = line.split('  ', 1)
        if Path(name).name!=name or sha256((output/name).read_bytes())!=expected:
            raise ValueError(f'Checksum mismatch: {name}')


def npm_command():
    if os.name=='nt':
        node = Path(shutil.which('node') or '')
        return [str(node), str(node.parent/'node_modules/npm/bin/npm-cli.js')]
    return ['npm']


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--version')
    parser.add_argument('--channel', choices=('dev', 'stable'), default='dev')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    info = metadata(root, args.version, args.channel)
    build_env = {**os.environ, 'ER_BUILD_VERSION': info['version'], 'ER_BUILD_CHANNEL': info['channel'],
                 'ER_BUILD_BRANCH': info['branch'], 'VITE_APP_VERSION': info['version'],
                 'SOURCE_DATE_EPOCH': str(info['source_date_epoch'])}
    for script in ('check_translations.py', 'check_game_translations.py'):
        subprocess.run([sys.executable, str(root/'scripts'/script)], cwd=root, env=build_env, check=True)
    subprocess.run([*npm_command(), 'run', 'build'], cwd=root/'frontend', env=build_env, check=True)
    subprocess.run([sys.executable, '-m', 'platformio', 'run', '--environment', HARDWARE], cwd=root, env=build_env, check=True)
    core = Path(os.environ.get('PLATFORMIO_CORE_DIR', Path.home()/'.platformio'))
    partitions = core/'packages/framework-arduinoespressif32/tools/partitions/default_16MB.csv'
    build = root/'.pio/build'/HARDWARE
    result = package(root, root/'dist', info, build/'firmware.bin', build/'firmware.elf', partitions, root/'frontend/dist')
    print(json.dumps(result, indent=2))


if __name__=='__main__':
    main()
