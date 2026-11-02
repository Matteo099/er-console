"""Inspect an LVGL UI and capture its actual RGB565 render over the USB-UART.

Requires UI_DIAGNOSTICS_ENABLED=1 and pyserial (included in PlatformIO's Python).
Does not flash firmware, access the SD directly or execute game scripts.
Example: python scripts/ui_capture.py --command "ui settings" --snapshot settings --tree
"""
import argparse
import re
import struct
import sys
import time
import zlib
from pathlib import Path

import serial

if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8', errors='backslashreplace')


def write_png(path, width, height, pixels):
    def chunk(kind, data):
        return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data) & 0xffffffff)
    scanlines = bytearray()
    for y in range(height):
        scanlines.append(0)
        for x in range(width):
            color = pixels[y * width + x]
            scanlines.extend((((color >> 11) & 31) * 255 // 31,
                              ((color >> 5) & 63) * 255 // 63,
                              (color & 31) * 255 // 31))
    data = b'\x89PNG\r\n\x1a\n'
    data += chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0))
    data += chunk(b'IDAT', zlib.compress(scanlines)) + chunk(b'IEND', b'')
    path.write_bytes(data)


class Probe:
    def __init__(self, port, output):
        self.port = serial.Serial()
        self.port.port = port
        self.port.baudrate = 115200
        self.port.timeout = 0.25
        self.port.dtr = False
        self.port.rts = False
        self.port.open()
        self.output = output
        self.output.mkdir(parents=True, exist_ok=True)
        self.log = []

    def read_until(self, predicate, timeout=30):
        started = time.monotonic()
        lines = []
        while time.monotonic() - started < timeout:
            raw = self.port.readline()
            if not raw:
                continue
            line = raw.decode('utf-8', errors='replace').strip()
            lines.append(line)
            if '@PX ' not in line:
                self.log.append(line)
            if predicate(line):
                return lines
        raise TimeoutError('Serial response timed out. Recent log:\n' + '\n'.join(self.log[-20:]))

    def command(self, value):
        self.port.write((value + '\n').encode())
        lines = self.read_until(lambda line: '@UI DONE ' + value in line)
        time.sleep(0.3)
        return lines

    def capture(self, name):
        lines = self.command('ui snapshot')
        width = height = 0
        encoded = bytearray()
        for line in lines:
            match = re.search(r'@SNAPSHOT BEGIN (\d+) (\d+)', line)
            if match:
                width, height = map(int, match.groups())
            match = re.search(r'@PX ([0-9a-f]+)', line)
            if match:
                encoded.extend(bytes.fromhex(match.group(1)))
        if not width or not height or len(encoded) % 3:
            raise ValueError('Incomplete snapshot frame')
        pixels = []
        for offset in range(0, len(encoded), 3):
            count, lo, hi = encoded[offset:offset + 3]
            pixels.extend([lo | (hi << 8)] * count)
        if len(pixels) != width * height:
            raise ValueError(f'Corrupt snapshot: {len(pixels)} pixels, expected {width * height}')
        path = self.output / (name + '.png')
        write_png(path, width, height, pixels)
        print(f'Saved {path} ({width}x{height})')

    def close(self, name):
        (self.output / (name + '.log')).write_text('\n'.join(self.log) + '\n', encoding='utf-8')
        self.port.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', default='COM3')
    parser.add_argument('--output', type=Path, default=Path('.pio/ui-previews'))
    parser.add_argument('--command', action='append', default=[])
    parser.add_argument('--snapshot')
    parser.add_argument('--tree', action='store_true')
    parser.add_argument('--reset', action='store_true')
    args = parser.parse_args()
    probe = Probe(args.port, args.output)
    name = args.snapshot or 'inspection'
    try:
        if args.reset:
            probe.port.reset_input_buffer()
            probe.port.setRTS(True)
            time.sleep(0.1)
            probe.port.setRTS(False)
            probe.read_until(lambda line: 'Entering MainMenuScene' in line, timeout=20)
            time.sleep(0.4)
        for command in args.command:
            lines = probe.command(command)
            print('\n'.join(line for line in lines if '@PX ' not in line))
        if args.tree:
            lines = probe.command('ui tree')
            text = '\n'.join(lines)
            (args.output / (name + '.tree.txt')).write_text(text, encoding='utf-8')
            print(text)
        if args.snapshot:
            probe.capture(name)
    finally:
        probe.close(name)


if __name__ == '__main__':
    main()
