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
# tft_src = os.path.join(project_dir, "include", "User_Setup.h")
fonts_src = os.path.join(project_dir, "include", "fonts")

# Destination paths (where libraries expect them)
lv_dst = os.path.join(libdeps, "lv_conf.h")
# tft_dst = os.path.join(libdeps, "TFT_eSPI", "User_Setup.h")

# LVGL font directory
lvgl_fonts_dst = os.path.join(libdeps, "lvgl", "src", "font")

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

def sync_directory(src_dir, dst_dir):
    if not os.path.isdir(src_dir):
        print(f"[WARN] Missing source directory: {src_dir}")
        return

    for root, dirs, files in os.walk(src_dir):
        rel_path = os.path.relpath(root, src_dir)
        dst_root = os.path.join(dst_dir, rel_path)

        for file in files:
            src_file = os.path.join(root, file)
            dst_file = os.path.join(dst_root, file)

            os.makedirs(dst_root, exist_ok=True)

            if not os.path.exists(dst_file) or not filecmp(src_file, dst_file):
                shutil.copy2(src_file, dst_file)
                print(f"[SYNC] {src_file} -> {dst_file}")

copy_if_exists(lv_src, lv_dst)
# copy_if_exists(tft_src, tft_dst)

# Sync all fonts
sync_directory(fonts_src, lvgl_fonts_dst)