Import("env")
import importlib.util
import re
import hashlib
from pathlib import Path

root = Path(env['PROJECT_DIR'])
spec = importlib.util.spec_from_file_location('ci_build_metadata', root/'scripts/ci/build_metadata.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
info = module.metadata(root)
# Quote branch names as data; Git branch names must not become compiler options.
branch = re.sub(r'[^A-Za-z0-9._/-]', '-', info['branch'])
env.Append(CPPDEFINES=[
    ('APP_VERSION', '\\"%s\\"' % info['version']),
    ('GIT_HASH', '\\"%s\\"' % info['commit'][:12]),
    ('GIT_BRANCH', '\\"%s\\"' % branch),
    ('DEV_MODE', 'true' if info['dev_mode'] else 'false'),
    ('OTA_HARDWARE_ID', '\\"%s\\"' % module.HARDWARE),
])
framework = Path(env.PioPlatform().get_package_dir('framework-arduinoespressif32'))
partition_name = env.GetProjectOption('board_build.partitions', env.BoardConfig().get('build.arduino.partitions', 'default.csv'))
partition_file = Path(partition_name)
if not partition_file.is_absolute():
    partition_file = root/partition_name if (root/partition_name).is_file() else framework/'tools/partitions'/partition_name
partition_hash = hashlib.sha256(partition_file.read_bytes()).hexdigest()
env.Append(CPPDEFINES=[('OTA_PARTITION_TABLE_SHA256', '\\"%s\\"' % partition_hash)])
print('\n========== BUILD INFO ==========')
for key in ('version', 'branch', 'commit', 'channel', 'dev_mode'):
    print(f'{key}: {info[key]}')
print('================================\n')
