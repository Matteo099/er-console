"""One version/commit/channel shared by firmware, portal and release bundles."""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess

SEMVER = re.compile(r'^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)(?:-([0-9A-Za-z-]+(?:\.[0-9A-Za-z-]+)*))?(?:\+([0-9A-Za-z-]+(?:\.[0-9A-Za-z-]+)*))?$')
HARDWARE = 'esp32-s3-devkitc-1-n16r8v'


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args], text=True).strip()


def valid_version(value):
    match = SEMVER.fullmatch(value)
    if not match or any(part.isdigit() and len(part)>1 and part[0]=='0'
                        for part in (match[4] or '').split('.')):
        raise ValueError(f'Invalid SemVer version: {value!r}')
    return value


def dev_version(base, run, attempt, commit):
    major, minor, patch = map(int, base.split('.'))
    return f'{major}.{minor}.{patch+1}-dev.{int(run)}.{int(attempt)}+sha.{commit[:12]}'


def metadata(root, version=None, channel=None, environ=None):
    env = os.environ if environ is None else environ
    commit = git(root, 'rev-parse', 'HEAD')
    dirty = bool(git(root, 'status', '--porcelain', '--untracked-files=no'))
    branch = env.get('ER_BUILD_BRANCH') or env.get('GITHUB_HEAD_REF') or env.get('GITHUB_REF_NAME') or git(root, 'branch', '--show-current') or 'detached'
    channel = channel or env.get('ER_BUILD_CHANNEL', 'dev')
    if channel not in ('stable', 'dev'):
        raise ValueError('Channel must be stable or dev')
    version = version or env.get('ER_BUILD_VERSION')
    if not version:
        tags = git(root, 'tag', '--merged', 'HEAD').splitlines()
        versions = [tuple(map(int, tag.removeprefix('v').split('.')))
                    for tag in tags if re.fullmatch(r'v?(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)', tag)]
        base = '.'.join(map(str, max(versions, default=(0, 0, 0))))
        if channel == 'stable':
            raise ValueError('Stable builds require an explicit Semantic Release version')
        version = dev_version(base, env.get('GITHUB_RUN_NUMBER', 0), env.get('GITHUB_RUN_ATTEMPT', 1), commit)
    valid_version(version)
    if channel == 'stable' and '-' in version.split('+')[0]:
        raise ValueError('A stable release cannot have a prerelease version')
    if channel == 'stable' and dirty:
        raise ValueError('A stable release requires a clean tracked source tree')
    if channel == 'dev' and '-' not in version.split('+')[0]:
        raise ValueError('Development builds require a prerelease version')
    if dirty and not version.endswith('.dirty'):
        version += '.dirty' if '+' in version else '+dirty'
    return {
        'version': version, 'channel': channel, 'commit': commit, 'branch': branch,
        'hardware': HARDWARE, 'dev_mode': channel != 'stable', 'dirty': dirty,
        'source_date_epoch': int(git(root, 'show', '-s', '--format=%ct', 'HEAD')),
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--version')
    parser.add_argument('--channel', choices=('dev', 'stable'), default='dev')
    parser.add_argument('--github-output', type=Path)
    args = parser.parse_args()
    result = metadata(Path(__file__).resolve().parents[2], args.version, args.channel)
    print(json.dumps(result, indent=2))
    if args.github_output:
        with args.github_output.open('a', encoding='utf-8') as target:
            target.write(f'version={result["version"]}\nhardware={result["hardware"]}\n')


if __name__ == '__main__':
    main()
