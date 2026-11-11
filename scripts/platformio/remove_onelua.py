import os
from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()

def remove_onelua():
    libdeps_dir = env.get("PROJECT_LIBDEPS_DIR")
    env_name = env.get("PIOENV")

    if not libdeps_dir or not env_name:
        print("[remove_onelua] Impossibile determinare PROJECT_LIBDEPS_DIR o PIOENV")
        return

    lua_dir = os.path.join(libdeps_dir, env_name, "lua")

    if not os.path.isdir(lua_dir):
        print(f"[remove_onelua] Directory Lua non trovata: {lua_dir}")
        return

    onelua_file = os.path.join(lua_dir, "onelua.c")

    if os.path.isfile(onelua_file):
        try:
            os.remove(onelua_file)
            print(f"[remove_onelua] Rimosso: {onelua_file}")
        except Exception as e:
            print(f"[remove_onelua] Errore rimuovendo {onelua_file}: {e}")
    else:
        print("[remove_onelua] onelua.c non trovato, nulla da fare")

remove_onelua()
