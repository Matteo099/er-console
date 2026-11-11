# Supporto editor Lua

`generate_lua_stubs.py` legge le annotazioni nei binding C++ e rigenera `lua_stubs/er_api.lua`, usato dall'editor per le API Lua.

```sh
python scripts/extension/generate_lua_stubs.py
```

Il comando parte dalla radice del repository. La configurazione VS Code lo richiama anche al salvataggio dei file `*Bindings.cpp`, se l'estensione Run on Save è installata.
