import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
import zipfile

sys.path.insert(0, str(Path(__file__).resolve().parent))
from build_bundle import package, verify, slot_size
from build_metadata import metadata, dev_version, valid_version, HARDWARE
import sd_pack


class BundleTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.git('init', '-b', 'main')
        self.git('config', 'user.name', 'CI test')
        self.git('config', 'user.email', 'ci@example.invalid')
        for name, data in {
            'sd/.core/audio/tickingclock.wav': b'wave-clock',
            'sd/.core/audio/lose.wav': b'wave-lose',
            'sd/.core/audio/win.wav': b'wave-win',
            'sd/.core/images/logo.png': b'png-logo',
            'sd/.core/config/wifi.cfg': b'ssid=PRIVATE\npassword=SECRET\n',
            'sd/.core/portal/index.html': b'STALE PORTAL',
            'sd/games/test/config.json': b'{}',
            'sd/games/test/game.lua': b'function init() end',
        }.items():
            self.write(name, data)
        self.git('add', '.')
        self.git('commit', '-m', 'feat: add test runtime')
        self.git('tag', 'v1.2.3')
        self.write('sd/user-save.json', b'PRIVATE SAVE')
        self.firmware = self.write('build/firmware.bin', b'\xe9firmware')
        self.elf = self.write('build/firmware.elf', b'\x7fELFsymbols')
        self.partitions = self.write('partitions.csv', b'# layout\napp0,app,ota_0,0x10000,0x640000,\napp1,app,ota_1,0x650000,0x640000,\n')
        self.write('frontend/dist/index.html', b'COMPILED PORTAL')
        self.write('frontend/dist/assets/app.js.gz', b'gzip-js')
        self.info = metadata(self.root, '1.3.0', 'stable', {})
        self.output = self.root/'dist'

    def git(self, *args):
        return subprocess.check_output(['git', '-C', str(self.root), *args], text=True, stderr=subprocess.DEVNULL).strip()

    def write(self, name, data):
        path = self.root/name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
        return path

    def bundle(self):
        return package(self.root, self.output, self.info, self.firmware, self.elf, self.partitions, self.root/'frontend/dist')

    def test_sd_includes_hidden_assets_and_compiled_portal_without_private_files(self):
        manifest = self.bundle()
        prefix = sd_pack.root(manifest['files']['resources']['sha256'])+'/'
        with zipfile.ZipFile(self.output/manifest['files']['sd']['file']) as archive:
            names = [name.removeprefix(prefix) for name in archive.namelist()]
            self.assertIn('.core/audio/win.wav', names)
            self.assertIn('.core/images/logo.png', names)
            self.assertIn('games/test/game.lua', names)
            self.assertIn('.core/portal/assets/app.js.gz', names)
            self.assertNotIn('.core/config/wifi.cfg', names)
            self.assertNotIn('user-save.json', names)
            self.assertEqual(archive.read(prefix+'.core/portal/index.html'), b'COMPILED PORTAL')
            self.assertEqual(archive.read(prefix+'.complete'), sd_pack.marker('1.3.0', manifest['files']['resources']['sha256'], HARDWARE))
            info = json.loads(archive.read(prefix+'build-info.json'))
            self.assertEqual(info['version'], '1.3.0')
            self.assertEqual(info['commit'], self.git('rev-parse', 'HEAD'))
        verify(self.output)
        self.assertEqual(manifest['schema_version'], 2)
        self.assertEqual(sd_pack.decode((self.output/manifest['files']['resources']['file']).read_bytes())['games/test/game.lua'], b'function init() end')

    def test_resources_cannot_change_after_hash_is_embedded_in_firmware(self):
        with self.assertRaisesRegex(ValueError, 'changed after firmware'):
            package(self.root, self.output, self.info, self.firmware, self.elf, self.partitions, self.root/'frontend/dist', '0'*64)

    def test_compiled_firmware_must_contain_the_pack_hash(self):
        manifest = self.bundle()
        digest = manifest['files']['resources']['sha256']
        with self.assertRaisesRegex(ValueError, 'does not contain'):
            package(self.root, self.output, self.info, self.firmware, self.elf, self.partitions, self.root/'frontend/dist', digest)
        self.firmware.write_bytes(b'\xe9firmware'+digest.encode())
        package(self.root, self.output, self.info, self.firmware, self.elf, self.partitions, self.root/'frontend/dist', digest)

    def test_corrupt_artifact_is_rejected(self):
        manifest = self.bundle()
        (self.output/manifest['files']['firmware']['file']).write_bytes(b'corrupted')
        with self.assertRaisesRegex(ValueError, 'checksum mismatch'):
            verify(self.output)

    def test_checksums_cover_manifest_and_release_changelog(self):
        self.write('dist/CHANGELOG.md', b'# Release notes\n')
        self.bundle()
        (self.output/'CHANGELOG.md').write_bytes(b'changed notes')
        with self.assertRaisesRegex(ValueError, 'Checksum mismatch'):
            verify(self.output)

    def test_invalid_and_oversize_firmware_cannot_be_packaged(self):
        self.firmware.write_bytes(b'not ESP firmware')
        with self.assertRaises(ValueError):
            self.bundle()
        self.partitions.write_text('app0,app,ota_0,0,1,\napp1,app,ota_1,1,1,\n')
        self.firmware.write_bytes(b'\xe9too large')
        with self.assertRaisesRegex(ValueError, 'exceeds OTA slot'):
            self.bundle()

    def test_missing_frontend_or_game_script_prevents_distribution(self):
        (self.root/'frontend/dist/index.html').unlink()
        with self.assertRaisesRegex(ValueError, 'frontend'):
            self.bundle()
        self.write('frontend/dist/index.html', b'compiled')
        (self.root/'sd/games/test/game.lua').unlink()
        with self.assertRaises(FileNotFoundError):
            self.bundle()

    def test_partition_layout_requires_both_slots(self):
        self.assertEqual(slot_size(self.partitions), 6553600)
        self.partitions.write_text('app0,app,ota_0,0,1,\n')
        with self.assertRaisesRegex(ValueError, 'two OTA slots'):
            slot_size(self.partitions)

    def test_dev_version_is_traceable_and_valid_semver(self):
        info = metadata(self.root, environ={'GITHUB_RUN_NUMBER':'123','GITHUB_RUN_ATTEMPT':'2','GITHUB_HEAD_REF':'feature/test'})
        self.assertEqual(info['version'], f'1.2.4-dev.123.2+sha.{info["commit"][:12]}')
        self.assertEqual(info['branch'], 'feature/test')
        self.assertTrue(info['dev_mode'])
        self.assertEqual(valid_version(info['version']), info['version'])
        self.assertEqual(dev_version('0.0.0', 1, 1, 'abc123'), '0.0.1-dev.1.1+sha.abc123')

    def test_stable_channel_is_explicit_even_in_detached_checkout(self):
        self.git('checkout', '--detach')
        info = metadata(self.root, '2.0.0', 'stable', {'ER_BUILD_BRANCH':'main'})
        self.assertFalse(info['dev_mode'])
        self.assertEqual(info['hardware'], HARDWARE)
        self.assertEqual(info['branch'], 'main')
        with self.assertRaises(ValueError): metadata(self.root, '2.0.0-dev.1', 'stable', {})
        with self.assertRaises(ValueError): metadata(self.root, '2.0.0', 'dev', {})
        with self.assertRaises(ValueError): metadata(self.root, channel='stable', environ={})

    def test_unsafe_version_strings_never_reach_shell_or_filenames(self):
        for value in ('1.0.0;echo unsafe', '1.0.0\nfoo', '01.0.0', '1.0.0-dev.01', '../1.0.0'):
            with self.subTest(value=value), self.assertRaises(ValueError): valid_version(value)

    def test_stable_build_rejects_dirty_sources_and_dev_marks_them(self):
        self.write('sd/games/test/game.lua', b'changed source')
        with self.assertRaisesRegex(ValueError, 'clean tracked source'):
            metadata(self.root, '1.3.0', 'stable', {})
        info = metadata(self.root, environ={})
        self.assertTrue(info['dirty'])
        self.assertTrue(info['version'].endswith('.dirty'))


if __name__=='__main__':
    unittest.main()
