# Dynamic hints and the game journal

The inventory button opens a journal with **Objects** and **Hints** tabs. Its
badge counts collected objects and unlocked hints. Each list opens a rich
text/image reader; the reader's list button returns to the same journal tab.
Locked hints are never listed.

A newly unlocked hint is illuminated over a dark background, then flies into
the journal button using the item discovery animation. After landing, its
content opens automatically. The timer and Lua game continue throughout the
journal, animation and reader: reading a hint does not pause the game.

## Authoring hints in Lua

Define the rules in `init()` and select the objective the players are currently
trying to complete:

```lua
function init()
    scene.setHints({
        {
            id = "door-symbols",
            step = "open-door",
            afterSeconds = 90,
            title = system.translate({it = "I simboli", en = "The symbols"}),
            image = "images/symbols.png", -- optional animation preview
            content = {
                {type = GameObjectContentType.TEXT,
                 value = system.translate({
                     it = "Confronta i simboli con la mappa.",
                     en = "Compare the symbols with the map."
                 })},
                {type = GameObjectContentType.IMAGE,
                 path = "images/symbols.png", width = 240, height = 160}
            }
        },
        {
            id = "door-answer", step = "open-door", afterSeconds = 180,
            title = "A more specific hint",
            content = {{type = GameObjectContentType.TEXT, value = "Start at the red symbol."}}
        }
    })
    scene.setStep("open-door")
end

-- Call when the door opens; pending open-door hints will no longer unlock.
function doorOpened()
    scene.setStep("find-key")
end
```

`id`, `title` and the timed rule's `step` must be nonempty strings. IDs must be
unique in `setHints`. `afterSeconds` is a finite nonnegative number; fractional
seconds are allowed, and zero unlocks on the next running update. Time is
**active play time since entering that step**, excluding pauses. Repeating
`setStep` with the same value does not restart the deadline. A different step
starts a fresh clock; `setStep("")` disables timed unlocking. Returning to an
earlier step starts its clock again, without relocking hints already earned.

Several hints may share a step with progressively longer delays. Due hints
unlock in deadline order. A step change cancels only future unlocks from the
previous step; already unlocked hints remain available for the game session.

`setHints` replaces the future rule definitions and updates unlocked content
with matching IDs. It preserves unlocked history and current step time. For
language changes, call it again with translated titles/content and the same
stable IDs. There are no hardcoded room steps or deadlines in the firmware.

For conditions beyond a single timed objective, use your game's Lua state and
unlock a hint directly:

```lua
if wrongAttempts >= 3 then
    scene.unlockHint({
        id = "combination-help", title = "Check the order",
        content = {{type = GameObjectContentType.TEXT, value = "Read from left to right."}}
    })
end
```

Both APIs use the GameObject content-block format. Images resolve relative to
the game directory; the first content image is used as the animation preview
when `image` is omitted. Text-only hints use a question mark. Reusing an ID
updates its content without duplicate entries or announcements.

## Lifecycle and implementation

Hints become accessible as soon as unlocked. Announcements wait while paused,
in settings, player selection or exit confirmation, or while another hint is
being read. They can appear over the timer, journal or object reader. Reading a
queued hint manually consumes its announcement. A game over stops the animation;
retry clears hints, deadlines and viewed counts. Pause/resume hardware checks
preserve both objects and hints.

Each hint's first read, including automatic opening, contributes once to the
victory screen's existing hints-used counter. Reopening it or paging back does
not increase that count. Hint history belongs to the current game and is not
persisted as personal settings.

`GameHints` owns the deadline/unlocked/viewed model. Lua posts commands through
the scene mailbox; `GameScene` evaluates deadlines using `GameSessionClock` and
`GameView` coordinates `GameJournalSubview`, `ObjectDiscoverySubview` and the
existing rich reader. LVGL work stays on the UI task. The reader builds image
widgets only when opened; only one reveal is animated at a time.

The bilingual `5-t_gameobject` demo includes staged hints. Copy its updated
`game.lua` from `sd/games/5-t_gameobject/` to the SD to use it; uploading firmware
alone does not replace an existing SD game script.

## Verification

With `UI_DIAGNOSTICS_ENABLED=1`:

```powershell
python scripts/diagnostics/ui_hints.py --port COM4
python scripts/diagnostics/ui_object_discovery.py --port COM4
python scripts/diagnostics/ui_game_translations.py --port COM4
python scripts/diagnostics/ui_ingame_settings.py --port COM4
```

The hints probe covers deadlines, skipped steps, pause time, localization,
deduplication, invalid Lua input, automatic rich-content opening, deferred
notifications, retry and unique usage counts. Real TFT previews cover locked
and unlocked lists, tabs, scrolling and reader navigation in Italian/English,
without starting games, driving room hardware or writing statistics.

Verified on ESP32-S3 on 18 November 2026: **34 hint/model/Lua checks**, **21
object-discovery checks** and **13 in-game settings checks** passed. Real TFT
captures verify both languages, automatic rich-content opening, journal tabs,
eight-entry scrolling and repeated reads. All five bilingual Lua fixtures pass,
including the demo's configured hints and step transitions. The real Loop game
also passed live settings changes, pause/resume and natural timeout with user
preferences and statistics restored. Debug and release builds pass.
The normal debug firmware was uploaded on COM4 with diagnostics disabled;
the final boot initialized SD/UI successfully and obtained a Wi-Fi address.

Logs/screenshots are under `.pio/hints-verified/`, `.pio/hints-discovery-regression/`,
`.pio/hints-lua-games/` and `.pio/hints-ingame-regression/`. No room hardware was
driven by the preview/fixture probes, and no SD game scripts were overwritten.
