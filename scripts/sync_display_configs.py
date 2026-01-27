Import("env")
import os
import shutil

project_dir = env["PROJECT_DIR"]
env_name = env["PIOENV"]

libdeps = os.path.join(project_dir, ".pio", "libdeps", env_name)

# Source files (versioned)
lv_src = os.path.join(project_dir, "include", "lv_conf.h")
tft_src = os.path.join(project_dir, "include", "User_Setup.h")

# Destination paths (where libraries expect them)
lv_dst = os.path.join(libdeps, "lv_conf.h")
tft_dst = os.path.join(libdeps, "TFT_eSPI", "User_Setup.h")

def copy_if_exists(src, dst):
    if not os.path.exists(src):
        print(f"[WARN] Missing source file: {src}")
        return

    os.makedirs(os.path.dirname(dst), exist_ok=True)

    if not os.path.exists(dst) or not filecmp(src, dst):
        shutil.copy2(src, dst)
        print(f"[SYNC] {src} -> {dst}")

def filecmp(src, dst):
    try:
        return open(src, "rb").read() == open(dst, "rb").read()
    except:
        return False

copy_if_exists(lv_src, lv_dst)
copy_if_exists(tft_src, tft_dst)
