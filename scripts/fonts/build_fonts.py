"""Regenerate the shared TTF/WOFF2 and three uncompressed LVGL heading fonts."""
import argparse
from pathlib import Path
import os
import shutil
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--node', default='node')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    converter = Path('scripts/fonts/node_modules/lv_font_conv/lv_font_conv.js')
    if not (root / converter).exists():
        parser.error('Install the pinned converter with: npm ci --prefix scripts/fonts')
    environment = os.environ.copy()
    local_tools = root / '.pio/font-tools'
    if local_tools.exists():
        environment['PYTHONPATH'] = str(local_tools) + os.pathsep + environment.get('PYTHONPATH', '')
    subprocess.run([sys.executable, 'scripts/fonts/make_escaperiom_font.py', '--output-dir', 'assets/fonts'],
                   cwd=root, env=environment, check=True)
    for size in (18, 22, 26):
        subprocess.run([args.node, str(converter), '--font', 'assets/fonts/EscaperiomDisplay.ttf',
                        '--size', str(size), '--bpp', '4', '--range',
                        '0x20-0x7e,0xa0-0xff,0x2018-0x2019,0x2013-0x2014,0x20ac',
                        '--format', 'lvgl', '--lv-include', 'lvgl.h', '--no-compress', '--lv-font-name', f'lv_font_escaperiom_{size}',
                        '--lv-fallback', 'lv_font_montserrat_14', '-o', f'include/fonts/lv_font_escaperiom_{size}.c'],
                       cwd=root, check=True)
    destination = root / 'frontend/src/assets/fonts'
    destination.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(root / 'assets/fonts/EscaperiomDisplay.woff2', destination / 'EscaperiomDisplay.woff2')


if __name__ == '__main__':
    main()
