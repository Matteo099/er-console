Import("env")
import subprocess
from datetime import datetime

def get_git_hash():
    try:
        return subprocess.check_output(
            ["git", "rev-parse", "--short", "HEAD"]
        ).decode().strip()
    except:
        return "unknown"

git_hash = get_git_hash()

# Get version from build flags
version = "unknown"
for flag in env.get("BUILD_FLAGS", []):
    if "APP_VERSION" in flag:
        version = flag.split("=")[1].replace('\\"', '').replace('"', '')

env.Append(
    CPPDEFINES=[
        ("GIT_HASH", '\\"%s\\"' % git_hash)
    ]
)

print("\n========== BUILD INFO ==========")
print(f"Version : {version}")
print(f"Git     : {git_hash}")
print(f"Time    : {datetime.now()}")
print("================================\n")