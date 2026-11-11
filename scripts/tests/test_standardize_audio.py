import contextlib
import importlib.util
import io
import json
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest
import wave

SCRIPT = Path(__file__).resolve().parents[1] / 'audio' / 'standardize_audio.py'
spec = importlib.util.spec_from_file_location('standardize_audio', SCRIPT)
audio = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audio)


class StandardizeAudioTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='console audio ')
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name).resolve()
        self.input = self.root / 'input'
        self.input.mkdir()

    def wav(self, name='sound.wav', rate=22050, channels=1, width=2, frames=2205):
        path = self.input / name
        path.parent.mkdir(parents=True, exist_ok=True)
        with wave.open(str(path), 'wb') as out:
            out.setnchannels(channels)
            out.setsampwidth(width)
            out.setframerate(rate)
            sample = b'\x80' if width == 1 else struct.pack('<h', 1000)
            out.writeframes(sample*frames*channels)
        return path

    def test_canonical_parameters_and_bitrate(self):
        result = audio.inspect_wav(self.wav())
        self.assertFalse(result['issues'])
        self.assertEqual((result['sample_rate'], result['channels'], result['bits_per_sample'], result['bitrate']), (22050, 1, 16, 352800))
        self.assertAlmostEqual(result['duration_seconds'], .1)

    def test_wrong_rate_channels_and_width_require_conversion(self):
        result = audio.inspect_wav(self.wav(rate=8000, channels=2, width=1, frames=800))
        self.assertEqual(len(result['issues']), 3)

    def test_truncated_chunk_and_incorrect_riff_length_rejected(self):
        path = self.wav()
        original = path.read_bytes()
        path.write_bytes(original[:-1])
        with self.assertRaises(audio.AudioError):
            audio.inspect_wav(path)
        changed = bytearray(original)
        struct.pack_into('<I', changed, 40, 0xffffffff)
        path.write_bytes(changed)
        with self.assertRaises(audio.AudioError):
            audio.inspect_wav(path)

    def test_byte_rate_and_memory_limit_validated(self):
        path = self.wav()
        self.assertTrue(audio.inspect_wav(path, max_bytes=128)['issues'])
        self.assertFalse(audio.inspect_wav(path, max_bytes=0)['issues'])
        data = bytearray(path.read_bytes())
        struct.pack_into('<I', data, 28, 0)
        path.write_bytes(data)
        self.assertTrue(any('alignment' in issue for issue in audio.inspect_wav(path)['issues']))

    def test_hidden_directories_and_nested_outputs(self):
        path = self.wav('.core/audio/sound.wav')
        output = self.input / 'converted'
        output.mkdir()
        shutil.copyfile(path, output / 'old.wav')
        self.assertEqual(audio.discover(self.input, output), [path])

    def test_report_cannot_overwrite_source_or_generated_wav(self):
        path = self.wav()
        before = path.read_bytes()
        output = self.root / 'output'
        for report in (path, output / 'sound.wav'):
            with contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(audio.main([str(self.input), '--output', str(output), '--report', str(report)]), 2)
            self.assertEqual(path.read_bytes(), before)
            self.assertFalse(output.exists())

    def test_collision_rejected_before_writing(self):
        path = self.wav()
        other = path.with_suffix('.mp3')
        other.write_bytes(b'not important for planning')
        output = self.root / 'output'
        with self.assertRaises(audio.AudioError):
            audio.plan_outputs(self.input, [path, other], output)
        self.assertFalse(output.exists())

    def test_source_never_replaced_even_with_overwrite(self):
        path = self.wav()
        before = path.read_bytes()
        with self.assertRaises(audio.AudioError):
            audio.plan_outputs(self.input, [path], self.input, overwrite=True)
        with self.assertRaises(audio.AudioError):
            audio.convert(path, path, overwrite=True)
        self.assertEqual(path.read_bytes(), before)

    def test_valid_sources_copy_without_ffmpeg_and_output_overwrite_is_explicit(self):
        path = self.wav()
        target = self.root / 'out/sound.wav'
        result, action = audio.convert(path, target, ffmpeg='definitely-not-installed')
        self.assertEqual(action, 'copied')
        self.assertFalse(result['issues'])
        with self.assertRaises(audio.AudioError):
            audio.plan_outputs(self.input, [path], target.parent)
        self.assertEqual(path.read_bytes(), target.read_bytes())

    def test_cli_report_and_exit_status(self):
        self.wav(rate=8000)
        report = self.root / 'report.json'
        with contextlib.redirect_stdout(io.StringIO()):
            status = audio.main([str(self.input), '--check', '--report', str(report)])
        self.assertEqual(status, 1)
        self.assertEqual(json.loads(report.read_text())['failed'], 1)

    @unittest.skipUnless(shutil.which('ffmpeg'), 'FFmpeg is optional for validation')
    def test_actual_stereo_8bit_conversion_preserves_duration(self):
        path = self.wav(rate=8000, channels=2, width=1, frames=800)
        original = path.read_bytes()
        result, action = audio.convert(path, self.root/'output/sound.wav')
        self.assertEqual(action, 'converted')
        self.assertFalse(result['issues'])
        self.assertAlmostEqual(result['duration_seconds'], .1, places=4)
        self.assertEqual(path.read_bytes(), original)

    @unittest.skipUnless(shutil.which('ffmpeg'), 'FFmpeg is optional for validation')
    def test_actual_mp3_conversion(self):
        path = self.wav()
        mp3 = self.input/'music.mp3'
        subprocess.run(['ffmpeg', '-v', 'error', '-nostdin', '-i', str(path), str(mp3)], check=True, capture_output=True)
        result, action = audio.convert(mp3, self.root/'output/music.wav')
        self.assertEqual(action, 'converted')
        self.assertFalse(result['issues'])
        self.assertGreater(result['frames'], 0)

    @unittest.skipUnless(shutil.which('ffmpeg'), 'FFmpeg is optional for validation')
    def test_failed_conversion_does_not_replace_destination(self):
        bad = self.input/'broken.mp3'
        bad.write_bytes(b'broken')
        target = self.root/'out/sound.wav'
        target.parent.mkdir()
        target.write_bytes(b'original output')
        with self.assertRaises(audio.AudioError):
            audio.convert(bad, target, overwrite=True)
        self.assertEqual(target.read_bytes(), b'original output')
        self.assertEqual(list(target.parent.glob('.audio-*')), [])


if __name__ == '__main__':
    unittest.main()
