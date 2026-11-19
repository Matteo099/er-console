# Gradual item discovery

Discovered GameObjects appear over a dark scrim, illuminated by the current
theme's pastel accent. After a 650 ms reveal they shrink and arc into the
inventory button over 500 ms. The object becomes browsable only on landing.
The game clock and Lua updates continue during this presentation.

The inventory button opens the [Objects/Hints journal](GAME_HINTS.md), including
when both lists are empty. Its badge counts collected objects and unlocked hints.
Discoveries queue in authored order. They wait until the game is running and
the timer view is visible; settings, the item viewer, player selection and exit
confirmation take priority. Interruptions leave uncollected items queued.
Game over dismisses the animation; retry clears the inventory and queue.

## Lua API

Discover an item when a puzzle or interaction reveals it:

```lua
scene.discoverGameObject({
    id = "laboratory-map",
    title = system.translate({it = "Mappa", en = "Map"}),
    image = "images/map.png",
    content = {
        {type = GameObjectContentType.TEXT, value = "A clue..."},
        {type = GameObjectContentType.IMAGE, path = "images/map.png"}
    }
})
```

`id` is a stable identifier: reuse it to update an item's translated title or
content without another reward. Without `id`, the title is the identity, so
localized objects should always provide an ID. `image` is optional: the first
content image is used otherwise; a file/prop icon is shown when no image can be
loaded. Relative image paths resolve below the game's directory. Content blocks
retain array order. Only one animated image/widget tree is allocated at a time;
the viewer defers image construction until it is opened.

`scene.setGameObjects(objects)` still replaces the desired collection. Newly
introduced IDs are animated; collected IDs update in place; absent IDs are
removed, including any matching in-flight reveal. Duplicate IDs are coalesced.
An empty array clears the inventory. Rapid incremental calls between UI frames
are retained, while a later replacement snapshot supersedes earlier updates.

Do not publish every room object at initialization if they should be secret.
Publish only discoveries as progress occurs. The object demo starts empty,
reveals its map after two seconds of active play and its board after six.
Its snapshots retain stable IDs across language changes, including while paused.
Deploy the updated demo script from `sd/games/5-t_gameobject/game.lua` to the SD
to see that authored progression; existing scripts can use the new API directly.

## Implementation and verification

`GameObjectInventory` owns discovery/collection state. `GameView` coordinates it
with `ObjectDiscoverySubview`; `GameObjectSubview` browses only collected items.
Lua posts updates through the scene's mutex-protected mailbox. All LVGL work is
performed on the UI task. The animation is advanced by `update()` at up to 30
frames per second, using a small transformed item region and a static scrim
instead of repeatedly fading the whole SPI display. There are no animation
timers/callbacks referencing a destroyed view.

With diagnostic firmware:

```powershell
python scripts/diagnostics/ui_object_discovery.py --port COM4
python scripts/diagnostics/ui_game_translations.py --port COM4
```

The first probe checks ordering, deduplication, localization, landing timing,
pause/modal deferral, removal, retry, Lua mailbox dispatch and screenshots in
IT/EN. Preview scenes do not execute games, operate room hardware or modify
statistics/preferences. Frozen reveal/flight frames are diagnostic-only.
The second probe executes the authored demo in a separate Lua VM with mocked
inputs, verifying progressive discovery and language changes while paused.

Verified on ESP32-S3 on 18 November 2026: **21 discovery/model/Lua checks** and
**13 in-game settings checks** passed. Real TFT captures cover reveal, flight,
missing-image fallback and two sequential landings in IT/EN. All five bilingual
Lua fixtures passed, including the demo's empty start, two/six-second discovery
steps and paused language switch. The real Loop game's settings, pause/resume
and natural timeout also passed with user preferences/statistics preserved.
Debug and release builds succeeded. Logs/screenshots are in
`.pio/discovery-verified/`, `.pio/discovery-lua-games/` and
`.pio/discovery-ingame-regression/`. No room hardware was driven by the preview
or fixture probes, and no SD game files were overwritten during verification.
