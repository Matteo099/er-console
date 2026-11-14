import hashlib
import struct
import unittest

import sd_pack


class SdPackTests(unittest.TestCase):
    def setUp(self):
        self.files = {'games/custom/game.lua': b'print("hello")',
                      '.core/portal/index.html': b'portal', 'build-info.json': b'{}'}
        self.packed = sd_pack.encode(self.files)

    def test_deterministic_roundtrip_and_case_insensitive_order(self):
        self.assertEqual(sd_pack.decode(self.packed), self.files)
        self.assertEqual(self.packed, sd_pack.encode(dict(reversed(list(self.files.items())))))
        files = {'games/A': b'a', 'games/_': b'b', 'games/b': b'c'}
        self.assertEqual(sd_pack.decode(sd_pack.encode(files)), files)

    def test_no_user_paths_traversal_or_fat_aliases(self):
        for path in ('../games/a', '/games/a', 'games/../a', 'games//a', 'games/a/',
                     'games/a.', 'games/a ', 'games/a:b', 'games/a\\b', 'games/\x00a',
                     '.core/config/settings', 'statistics/data', 'games/'+'a'*155):
            with self.subTest(path=path), self.assertRaises(ValueError):
                sd_pack.encode({path: b'data'})
        with self.assertRaises(ValueError):
            sd_pack.encode({'games/A': b'a', 'games/a': b'b'})

    def test_truncated_or_extra_data_never_validates(self):
        for length in (0, 8, 11, 12, 25, len(self.packed)-1):
            with self.subTest(length=length), self.assertRaises(ValueError):
                sd_pack.decode(self.packed[:length])
        with self.assertRaises(ValueError):
            sd_pack.decode(self.packed+b'extra')

    def test_content_corruption_and_header_corruption_are_rejected(self):
        bad = bytearray(self.packed)
        bad[-1] ^= 1
        with self.assertRaisesRegex(ValueError, 'checksum'):
            sd_pack.decode(bad)
        bad = sd_pack.MAGIC+struct.pack('<I', 1025)
        with self.assertRaises(ValueError):
            sd_pack.decode(bad)

    def test_generation_marker_binds_version_hardware_and_content(self):
        digest = hashlib.sha256(self.packed).hexdigest()
        self.assertEqual(sd_pack.root(digest), '.releases/'+digest)
        self.assertEqual(sd_pack.marker('1.2.3', digest, 'board'), f'1.2.3\n{digest}\nboard\n'.encode())
        for value in ('../other', 'A'*64, ''):
            with self.assertRaises(ValueError): sd_pack.root(value)


if __name__ == '__main__':
    unittest.main()
