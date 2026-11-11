#!/usr/bin/env python3
"""Validate/convert console audio, independently of PlatformIO (Python 3.9+).

Default: recursively check sd/. Conversion requires --output and FFmpeg only
for non-standard sources. Original files are never overwritten.
"""
import argparse
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile

SAMPLE_RATE = 22050
CHANNELS = 1
BITS = 16
BITRATE = SAMPLE_RATE * CHANNELS * BITS
MAX_BYTES = 4 * 1024 * 1024
EXTENSIONS = {'.wav', '.mp3', '.flac', '.ogg', '.opus', '.m4a', '.aac', '.aif', '.aiff', '.wma', '.caf', '.w64'}


class AudioError(Exception):
    pass


def within(path, root):
    try:
        path.resolve().relative_to(root.resolve())
        return True
    except ValueError:
        return False


def inspect_wav(path, max_bytes=MAX_BYTES):
    """Read RIFF structure and PCM parameters without decoding or loading audio."""
    path = Path(path)
    size = path.stat().st_size
    fmt = None
    payload = None
    with path.open('rb') as source:
        header = source.read(12)
        if len(header) != 12 or header[:4] != b'RIFF' or header[8:] != b'WAVE':
            raise AudioError('not a RIFF/WAVE file')
        if struct.unpack_from('<I', header, 4)[0] + 8 != size:
            raise AudioError('RIFF length does not match file size (truncated or trailing data)')
        offset = 12
        while offset < size:
            if size-offset < 8:
                raise AudioError('incomplete chunk header')
            source.seek(offset)
            name, count = struct.unpack('<4sI', source.read(8))
            if count > size-offset-8:
                raise AudioError('truncated chunk payload')
            if name == b'fmt ':
                if fmt is not None or count < 16:
                    raise AudioError('duplicate or invalid fmt chunk')
                fmt = struct.unpack('<HHIIHH', source.read(16))
            if name == b'data':
                if payload is not None:
                    raise AudioError('multiple data chunks are not supported')
                payload = count
            offset += 8 + count + (count & 1)
            if offset > size:
                raise AudioError('missing odd-chunk padding')
    if fmt is None or payload is None:
        raise AudioError('missing fmt/data chunk')
    codec, channels, rate, byte_rate, alignment, bits = fmt
    issues = []
    if codec != 1:
        issues.append(f'codec must be PCM integer (found WAV format {codec})')
    if channels != CHANNELS:
        issues.append(f'channels must be {CHANNELS} (found {channels})')
    if rate != SAMPLE_RATE:
        issues.append(f'sample rate must be {SAMPLE_RATE} Hz (found {rate})')
    if bits != BITS:
        issues.append(f'sample width must be {BITS} bits (found {bits})')
    if not alignment or alignment != channels * bits // 8 or byte_rate != rate * alignment:
        issues.append('invalid block alignment/byte rate')
    if not payload or not alignment or payload % alignment:
        issues.append('empty or incomplete PCM frames')
    if max_bytes and size > max_bytes:
        issues.append(f'file exceeds console cache budget ({size} > {max_bytes} bytes)')
    return {
        'codec': 'pcm_s16le' if codec == 1 and bits == 16 else f'wav-format-{codec}/{bits}-bit',
        'sample_rate': rate, 'channels': channels, 'bits_per_sample': bits,
        'bitrate': byte_rate * 8, 'data_bytes': payload, 'file_bytes': size,
        'frames': payload // alignment if alignment else 0,
        'duration_seconds': payload / byte_rate if byte_rate else None,
        'issues': issues,
    }


def discover(source, output=None):
    source = Path(source).resolve()
    if not source.exists():
        raise AudioError(f'input does not exist: {source}')
    files = [source] if source.is_file() else source.rglob('*')
    result = sorted(path for path in files if path.is_file() and path.suffix.lower() in EXTENSIONS
                    and not (output and within(path, output)))
    if not result:
        raise AudioError('no audio files found')
    return result


def plan_outputs(source, files, output, overwrite=False):
    source, output = Path(source).resolve(), Path(output).resolve()
    base = source.parent if source.is_file() else source
    if output == base:
        raise AudioError('output must differ from the input directory; originals are preserved')
    inputs = {str(path.resolve()).casefold() for path in files}
    destinations = set()
    plan = []
    for path in files:
        destination = output / path.relative_to(base).with_suffix('.wav')
        key = str(destination.resolve()).casefold()
        if not within(destination, output) or key in inputs:
            raise AudioError(f'output would overwrite an input or escape output directory: {destination}')
        if key in destinations:
            raise AudioError(f'output collision (same stem in different audio formats): {destination}')
        if destination.exists() and not overwrite:
            raise AudioError(f'output exists: {destination}; use --overwrite for generated outputs')
        destinations.add(key)
        plan.append((path, destination))
    return plan


def convert(source, destination, ffmpeg='ffmpeg', overwrite=False, max_bytes=MAX_BYTES):
    if Path(source).resolve() == Path(destination).resolve():
        raise AudioError('conversion never overwrites an original input')
    destination.parent.mkdir(parents=True, exist_ok=True)
    fd, temporary = tempfile.mkstemp(prefix='.audio-', suffix='.wav', dir=destination.parent)
    os.close(fd)
    temporary = Path(temporary)
    try:
        try:
            original = inspect_wav(source, max_bytes)
            copy = not original['issues']
        except (AudioError, OSError):
            copy = False
        if copy:
            shutil.copyfile(source, temporary)
        else:
            command = [str(ffmpeg), '-v', 'error', '-nostdin', '-y', '-i', str(source),
                       '-map', '0:a:0', '-vn', '-sn', '-dn', '-ac', str(CHANNELS),
                       '-ar', str(SAMPLE_RATE), '-c:a', 'pcm_s16le', '-sample_fmt', 's16',
                       '-map_metadata', '-1', '-bitexact', '-f', 'wav', str(temporary)]
            try:
                completed = subprocess.run(command, capture_output=True, text=True, encoding='utf-8', errors='replace')
            except FileNotFoundError as error:
                raise AudioError('FFmpeg not found; install it/on PATH or pass --ffmpeg PATH') from error
            if completed.returncode:
                raise AudioError('FFmpeg conversion failed: '+completed.stderr.strip())
        result = inspect_wav(temporary, max_bytes)
        if result['issues']:
            raise AudioError('converted output is incompatible: '+'; '.join(result['issues']))
        if destination.exists() and not overwrite:
            raise AudioError(f'output exists: {destination}')
        os.replace(temporary, destination)
        return result, 'copied' if copy else 'converted'
    finally:
        temporary.unlink(missing_ok=True)


def run(args):
    source = args.input.resolve()
    output = args.output.resolve() if args.output else None
    if args.check and output:
        raise AudioError('--check cannot be combined with --output')
    files = discover(source, output)
    plan = plan_outputs(source, files, output, args.overwrite) if output else [(path, None) for path in files]
    if args.report:
        protected = {str(path.resolve()).casefold() for pair in plan for path in pair if path is not None}
        if str(args.report.resolve()).casefold() in protected:
            raise AudioError('report must not overwrite an input or generated audio file')
    records = []
    for path, destination in plan:
        record = {'input': str(path)}
        try:
            if destination:
                info, action = convert(path, destination, args.ffmpeg, args.overwrite, args.max_bytes)
                record.update(output=str(destination), action=action, **info)
            else:
                info = inspect_wav(path, args.max_bytes)
                record.update(action='valid' if not info['issues'] else 'invalid', **info)
            print(f"{record['action'].upper()}: {path}")
            for issue in record['issues']:
                print('  '+issue)
        except (AudioError, OSError, struct.error) as error:
            record.update(action='invalid', issues=[str(error)])
            print(f'INVALID: {path}\n  {error}')
        records.append(record)
    failed = sum(bool(record['issues']) for record in records)
    report = {
        'target': {'codec': 'pcm_s16le', 'sample_rate': SAMPLE_RATE, 'channels': CHANNELS,
                   'bits_per_sample': BITS, 'bitrate': BITRATE, 'max_file_bytes': args.max_bytes},
        'files': records, 'checked': len(records), 'failed': failed,
        'standardized_bytes': sum(record.get('file_bytes', 0) for record in records if not record['issues']),
    }
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(report, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
    print(f'{len(records)-failed}/{len(records)} compatible: PCM16 LE, {SAMPLE_RATE} Hz, mono, {BITRATE} bit/s')
    return 1 if failed else 0


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input', nargs='?', type=Path, default=Path('sd'))
    parser.add_argument('--check', action='store_true', help='validate only (default without --output)')
    parser.add_argument('--output', type=Path, help='convert/copy into a separate tree, retaining relative paths')
    parser.add_argument('--overwrite', action='store_true', help='allow replacement of generated outputs, never source files')
    parser.add_argument('--ffmpeg', default='ffmpeg', help='FFmpeg executable for conversions')
    parser.add_argument('--max-bytes', type=int, default=MAX_BYTES, help='max bytes per WAV; 0 checks format only (runtime cache remains bounded)')
    parser.add_argument('--report', type=Path, help='write a JSON validation/conversion report')
    args = parser.parse_args(argv)
    if args.max_bytes < 0 or 0 < args.max_bytes < 44:
        parser.error('--max-bytes must be 0 or at least 44')
    try:
        return run(args)
    except (AudioError, OSError) as error:
        print('ERROR: '+str(error), file=sys.stderr)
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
