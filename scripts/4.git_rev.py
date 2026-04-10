Import("env")
import subprocess
from datetime import datetime

def run(cmd):
    try:
        return subprocess.check_output(cmd).decode().strip()
    except:
        return None

def get_git_hash():
    return run(["git", "rev-parse", "--short", "HEAD"]) or "unknown"

def get_branch():
    branch = run(["git", "rev-parse", "--abbrev-ref", "HEAD"])
    return branch or "unknown"

git_hash = get_git_hash()
branch = get_branch()

# DEV MODE RULE
is_dev = branch != "main"

dev_flag = "true" if is_dev else "false"

# Get version from build flags
version = "unknown"
for flag in env.get("BUILD_FLAGS", []):
    if "APP_VERSION" in flag:
        version = flag.split("=")[1].replace('\\"', '').replace('"', '')

env.Append(
    CPPDEFINES=[
        ("GIT_HASH", '\\"%s\\"' % git_hash),
        ("GIT_BRANCH", '\\"%s\\"' % branch),
        ("DEV_MODE", dev_flag)
    ]
)

print("\n========== BUILD INFO ==========")
print(f"Version : {version}")
print(f"Branch  : {branch}")
print(f"Git     : {git_hash}")
print(f"DevMode : {dev_flag}")
print(f"Time    : {datetime.now()}")
print("================================\n")