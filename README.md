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

In the chooser:

- `[Clear Selection]` returns map selection to the game.
- The map you are in is marked `[Here]`; an explicitly chosen map `[Selected]`; both `[Here/Selected]`.
- Choosing a different map closes and rebuilds the world map with that world.
- B / Tab / Escape (the game's Cancel) closes the chooser.

## Features

- Finds every worldspace with usable world-map data, including maps added by other mods.
- Switches the map scene, camera, ordinary map markers, quest markers and the custom destination marker together.
- Fast travel across worldspaces through ordinary map markers.
- The chooser uses Skyrim's own message box - no SkyUI or other UI framework needed.
- Paginated map lists, duplicate-name disambiguation, persistent or one-shot selections.
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
| `MapsPerPage` | `6` | Worldspaces per chooser page, 1 through 7. |
| `ShowCancelButton` | `true` | Shows a Cancel button in the chooser. |
| `ShowClearSelectionButton` | `true` | Shows the option that returns map selection to the game. |
| `IncludedWorldspaces` | *(empty)* | Comma-separated EditorIDs allowed in the chooser; empty means all. |
| `ExcludedWorldspaces` | `Falskaar` | Comma-separated EditorIDs left out of the chooser; wins over the include list. |

The INI is read once at start-up; restart the game after changing it.

## Requirements and compatibility

- [SKSE64](https://skse.silverlock.org/) for your runtime, and
  [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444).
- Steam Skyrim 1.5.97, 1.6.318 to 1.6.1170, 1.7.99 and 1.7.104. VR and GOG are not supported.
- SkyUI is optional (only the button-bar hint needs it).
- Do not install it beside the original World Map Selector: both hook the same game code. If
  `WorldMapSelector.dll` is also loaded, this plugin logs an error and stays inactive.

## Known issues

- Falskaar's map is excluded by default: opening it remotely can stop later world transitions from completing.
- Map-interface replacements other than SkyUI's need wider testing.

Reports: include `Documents/My Games/Skyrim Special Edition/SKSE/WorldMapSelectorForController.log`, your
Skyrim version and a mod list.

## For testers and tools

DevBench / TestBench tool `wmsfc.control`: no arguments returns the live state (world map open, chooser
showing, trigger codes, how often each device opened the chooser, presses ignored outside the map, text
language); `{"op":"open"}` asks for the chooser through the same world-map-only check a press uses;
`{"op":"dismiss"}` closes it. Test steps: `TESTING.md`.

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
