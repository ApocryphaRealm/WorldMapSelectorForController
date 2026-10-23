# World Map Selector for Controller

Version 1.0.0

An SKSE plugin that lets you view and fast travel from world maps other than the world you are standing in -
open the Solstheim map from Skyrim, pick a Solstheim marker and travel there - with the map chooser opened by
one button from inside the world map. No hotkey to remember in gameplay: the button does nothing anywhere else.

## Using it

Open the world map, then press:

| Device | Default | INI key (`[Controls]`) |
| --- | --- | --- |
| Controller | **D-pad up** | `MapMenuButton=0x0001` |
| Keyboard | **C** | `MapMenuKey=0x2E` |

Neither is used by the vanilla world map. Outside the world map both presses are left to the game untouched.
With SkyUI's map interface, the button bar shows a `Select Map` hint with the controller button while a
controller is in use, and the keyboard key otherwise.

The maps are listed in a vertical panel down the right side of the world map (the side is a setting), as tall
as the map and scrollable, so every worldspace other mods add fits:

- D-pad up / down or the left stick moves the highlight, and the list scrolls to follow it; **A** chooses;
  **B** closes the panel and leaves the map open.
- On PC: the arrow keys or the mouse wheel move, Enter or a click chooses, Escape or Tab closes.
- `[Clear Selection]` returns map selection to the game.
- The map you are in is marked `[Here]`; an explicitly chosen map `[Selected]`; both `[Here/Selected]`.
- Choosing a different map closes and rebuilds the world map with that world.

The panel is drawn by the [Apocrypha Menu Framework](https://www.nexusmods.com/skyrimspecialedition/mods/190114) in its
theme, and while it is open the framework holds the controller, keyboard and mouse so the map behind it stays
still. Without the framework the plugin falls back to Skyrim's own message box (one row of maps per page).

## Features

- Finds every worldspace with usable world-map data, including maps added by other mods.
- Switches the map scene, camera, ordinary map markers, quest markers and the custom destination marker together.
- Fast travel across worldspaces through ordinary map markers.
- A scrollable side panel (Apocrypha Menu Framework), with a settings page for its side, width and text size.
- Duplicate-name disambiguation, persistent or one-shot selections.
- Text in eleven languages, following the game's language.
- No ESP and no Papyrus scripts.

## Configuration

`SKSE/Plugins/WorldMapSelectorForController.ini` documents every setting. Defaults:

| Setting | Default | Purpose |
| --- | --- | --- |
| `LogLevel` | `Info` | Log verbosity (`Trace`, `Debug`, `Info`, `Warn`, `Err`, `Critical`, `Off`). |
| `MapMenuKey` | `0x2E` (C) | DirectInput scan code that opens the chooser in the world map. `0x00` turns it off. |
| `MapMenuButton` | `0x0001` (D-pad up) | XInput button mask that opens the chooser in the world map. `0x0000` turns it off. |
| `PersistSelection` | `true` | Keeps the selected map after the world map closes. |
| `ShowMapMenuKeyHint` | `true` | Shows the chooser's key or button in SkyUI's world-map button bar. |
| `ShowMapMenuKeyHintOnLocalMap` | `false` | Also shows that hint on the local map. |
| `MapsPerPage` | `6` | Worldspaces per message-box page, 1 through 7 (message box only, without the framework). |
| `ShowCancelButton` | `true` | Shows a Cancel button in the message box (without the framework). |
| `ShowClearSelectionButton` | `true` | Shows the option that returns map selection to the game. |
| `IncludedWorldspaces` | *(empty)* | Comma-separated EditorIDs allowed in the chooser; empty means all. |
| `ExcludedWorldspaces` | `Falskaar` | Comma-separated EditorIDs left out of the chooser; wins over the include list. |
| `Side` (`[Panel]`) | `Right` | Which side of the world map the panel runs down: `Right` or `Left`. |
| `WidthPercent` (`[Panel]`) | `22` | Panel width as a percentage of the screen width, 15 through 40. |
| `TextScalePercent` (`[Panel]`) | `100` | Panel text size against the menu's normal size, 80 through 160. |

The INI is read at start-up. The settings page (Apocrypha Menu Framework, "World Map Selector for Controller")
changes the panel's side, width and text size and the `[Behavior]` switches live, and writes them to the INI.

## Requirements and compatibility

- [SKSE64](https://skse.silverlock.org/) for your runtime, and
  [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444).
- Steam Skyrim 1.5.97, 1.6.318 to 1.6.1170, 1.7.99 and 1.7.104. VR and GOG are not supported.
- Apocrypha Menu Framework for the side panel and the settings page (recommended). Without it the maps are
  offered in Skyrim's message box and the INI is the only settings surface.
- SkyUI is optional (only the button-bar hint needs it).
- Do not install it beside the original World Map Selector: both hook the same game code. If
  `WorldMapSelector.dll` is also loaded, this plugin logs an error and stays inactive.

## Known issues

- Falskaar's map is excluded by default: opening it remotely can stop later world transitions from completing.
- Map-interface replacements other than SkyUI's need wider testing.

Reports: include `Documents/My Games/Skyrim Special Edition/SKSE/WorldMapSelectorForController.log`, your
Skyrim version and a mod list.

## For testers and tools

DevBench / TestBench tool `wmsfc.control`: no arguments returns the live state (world map open, the panel's
rows, highlight, scroll position and rectangle, its side, width and text size, trigger codes, how often each
device opened it, presses ignored outside the map, text language); `{"op":"open"}` asks for the panel through
the same world-map-only check a press uses; `{"op":"dismiss"}` closes it; `{"op":"nav","rows":N}` and
`{"op":"select"}` drive it; `{"op":"settings","side":"left","width":25,"text":110}` sets and saves the page's
settings; `{"op":"fake","count":N}` (testing only) adds N rows the next time it opens, to prove it scrolls.
Test steps: `TESTING.md`.

## Building

Visual Studio 2022 or newer with the C++ workload (CMake and Ninja come with it), Git, and vcpkg with
`VCPKG_ROOT` set. Then `build.bat` (it runs `configure.bat` on the first build, which initializes the
CommonLibSSE NG submodule and lets vcpkg restore the dependencies). Output: `build/relwithdebinfo/`.

## Licence

Source authored for this project is released under the [Unlicense](UNLICENSE). The plugin binary links
CommonLibSSE NG statically, so the distributed DLL is governed by GPL-3.0-or-later with CommonLibSSE NG's
Modding and Linking Exceptions - see [LICENSING.md](LICENSING.md), [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)
and [NOTICE.md](NOTICE.md).

## Credit

Built on **World Map Selector** by **Apocrypher00**
([Nexus 187315](https://www.nexusmods.com/skyrimspecialedition/mods/187315),
[source](https://github.com/Apocrypher00/WorldMapSelector), version 1.4.0, released under the Unlicense).
The world-map switching, marker and fast-travel work is that author's. Also: CommonLibSSE NG and its
contributors, MinHook, and the libraries listed in THIRD_PARTY_NOTICES.md.
