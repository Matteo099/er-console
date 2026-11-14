"""Deterministic, streamable SD resources. No decompressor on the ESP32."""
import hashlib
import io
import struct

MAGIC = b'ERSDPK01'
MAX_FILES = 1024
MAX_PATH = 160
MAX_BYTES = 128 * 1024 * 1024


def valid_path(path):
    encoded = path.encode('utf-8')
    return (0 < len(encoded) <= MAX_PATH and len(path.split('/')) <= 12 and
            all(part not in ('', '.', '..') and not part.endswith(('.', ' ')) for part in path.split('/')) and
            not any(ord(char) < 32 or ord(char) >= 127 or char in '\\:*?"<>|' for char in path) and
            (path == 'build-info.json' or path.startswith(
                ('.core/audio/', '.core/images/', '.core/portal/', 'games/'))))


def encode(contents):
    if not 0 < len(contents) <= MAX_FILES:
        raise ValueError('Invalid SD pack entry count')
    stream = io.BytesIO()
    stream.write(MAGIC + struct.pack('<I', len(contents)))
    keys = [path.lower() for path in contents]
    if len(set(keys)) != len(keys):
        raise ValueError('Case-insensitive SD path collision')
    for path, data in sorted(contents.items(), key=lambda item: item[0].lower()):
        if not valid_path(path):
            raise ValueError(f'Unsafe SD resource path: {path}')
        name = path.encode('utf-8')
        stream.write(struct.pack('<HI', len(name), len(data)))
        stream.write(hashlib.sha256(data).digest())
        stream.write(name)
        stream.write(data)
    result = stream.getvalue()
    if len(result) > MAX_BYTES:
        raise ValueError('SD resource pack too large')
    return result


def decode(data):
    stream = io.BytesIO(data)
    if len(data) > MAX_BYTES or stream.read(8) != MAGIC:
        raise ValueError('Invalid SD pack header')

    def read(size):
        result = stream.read(size)
        if len(result) != size:
            raise ValueError('Truncated SD pack')
        return result

    count, = struct.unpack('<I', read(4))
    if not 0 < count <= MAX_FILES:
        raise ValueError('Invalid SD pack count')
    result = {}
    previous = ''
    for _ in range(count):
        length, size = struct.unpack('<HI', read(6))
        if not 0 < length <= MAX_PATH or size > MAX_BYTES:
            raise ValueError('Invalid SD pack entry size')
        digest = read(32)
        path = read(length).decode('utf-8')
        if not valid_path(path) or path.lower() <= previous:
            raise ValueError('Unsafe, duplicate or unordered SD pack path')
        content = read(size)
        if hashlib.sha256(content).digest() != digest:
            raise ValueError('SD resource checksum mismatch')
        result[path] = content
        previous = path.lower()
    if stream.read(1):
        raise ValueError('Trailing SD pack data')
    return result


def root(digest):
    if len(digest) != 64 or any(char not in '0123456789abcdef' for char in digest):
        raise ValueError('Invalid SD content hash')
    return '.releases/' + digest


def marker(version, digest, hardware):
    root(digest)
    if any(char in version + hardware for char in '\r\n'):
        raise ValueError('Invalid SD marker')
    return f'{version}\n{digest}\n{hardware}\n'.encode()


def inventory(contents):
    return (str(len(contents))+'\n'+''.join(f'{path}\t{len(data)}\n' for path, data in
            sorted(contents.items(), key=lambda item: item[0].lower()))).encode()
