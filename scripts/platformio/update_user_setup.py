"""
------------------------------------------------------------
Script: update_user_setup.py
Part of: PlatformIO ER-Console Project

Description:
    Reads hardware pin definitions from src/config/BoardConfig.h
    and updates the corresponding macros inside include/User_Setup.h.

    This ensures that the TFT_eSPI configuration always matches the
    board configuration without manually editing User_Setup.h.

Workflow:
    BoardConfig.h  --->  update_user_setup.py  --->  User_Setup.h
                                                     |
                                                     v
                                              sync_include_files.py
                                              copies it to .pio/libdeps

Author: Project automation
------------------------------------------------------------
"""

Import("env")
import os
import re

project_dir = env["PROJECT_DIR"]

board_config = os.path.join(project_dir, "src", "config", "BoardConfig.h")
user_setup = os.path.join(project_dir, "include", "User_Setup.h")


# Macros we want to extract from BoardConfig.h
PIN_MAP = {
    "TFT_CS_PIN": "TFT_CS",
    "TFT_DC_PIN": "TFT_DC",
    "TFT_RST_PIN": "TFT_RST",
    "SPI1_MISO_PIN": "TFT_MISO",
    "SPI1_MOSI_PIN": "TFT_MOSI",
    "SPI1_SCLK_PIN": "TFT_SCLK",
    "TOUCH_CS_PIN": "TOUCH_CS"
}


def extract_macros(file_path):
    """
    Parse #define macros from BoardConfig.h
    """
    macros = {}

    with open(file_path, "r") as f:
        for line in f:
            m = re.match(r"#define\s+(\w+)\s+([0-9\-]+)", line)
            if m:
                macros[m.group(1)] = m.group(2)

    return macros

def update_user_setup(file_path, board_values):
    """
    Replace macros inside User_Setup.h with board values,
    but ONLY if the line is not commented out.
    """

    updated_lines = []

    with open(file_path, "r") as f:
        lines = f.readlines()

    for line in lines:

        stripped = line.lstrip()

        # Skip commented lines
        if stripped.startswith("//"):
            updated_lines.append(line)
            continue

        replaced = False

        for board_key, user_key in PIN_MAP.items():

            if board_key not in board_values:
                continue

            # Match only real #define statements
            m = re.match(rf"\s*#define\s+{user_key}\s+([^\s]+)", line)

            if m:
                value = board_values[board_key]
                new_line = re.sub(
                    rf"#define\s+{user_key}\s+[^\s]+",
                    f"#define {user_key} {value}",
                    line
                )

                updated_lines.append(new_line)
                print(f"[UPDATE] {user_key} -> {value}")
                replaced = True
                break

        if not replaced:
            updated_lines.append(line)

    with open(file_path, "w") as f:
        f.writelines(updated_lines)


if not os.path.exists(board_config):
    print("[ERROR] BoardConfig.h not found")
    exit(1)

if not os.path.exists(user_setup):
    print("[ERROR] User_Setup.h not found")
    exit(1)


macros = extract_macros(board_config)
update_user_setup(user_setup, macros)

print("[DONE] User_Setup.h updated from BoardConfig.h")