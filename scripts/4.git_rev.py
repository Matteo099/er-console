Import("env")
import importlib.util
import re
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
])
print('\n========== BUILD INFO ==========')
for key in ('version', 'branch', 'commit', 'channel', 'dev_mode'):
    print(f'{key}: {info[key]}')
print('================================\n')
