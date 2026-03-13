"""
------------------------------------------------------------
Script: sync_include_files.py
Part of: PlatformIO ER-Console Project

Description:
    This script automates the synchronization of versioned configuration
    files from the project's 'include' directory to the PlatformIO
    library dependency directories. Specifically, it handles:

        - lv_conf.h       -> placed in the library path for LittlevGL (LVGL)
        - User_Setup.h    -> placed in the library path for TFT_eSPI

    The script checks if the source files exist, compares them with
    the destination copies, and only copies them if they differ,
    avoiding unnecessary overwrites.

Usage:
    Automatically executed as part of the PlatformIO build process
    by importing this script into the project's SConstruct environment.
------------------------------------------------------------
"""

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
