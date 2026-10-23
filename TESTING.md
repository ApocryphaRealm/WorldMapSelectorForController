# World Map Selector for Controller - in-game test steps (1.0.0)

Written 2026-10-23 because Njordlinger was busy when the build was packaged (lock held by another session, game
running). Run these in one combined round through Main Agent (rule 69).

## Build under test

- Package: `D:\Claude output\7. current test builds\Skyrim - World Map Selector for Controller\World Map Selector for Controller 1.0.0`
- Packaged DLL sha256 starts `E7FD36B12912CAC2`; check the installed DLL's hash matches before judging.

## Setup

1. `resolve.py run health-gate` passes; `resolve.py run agent-lock status` shows `instance:Njordlinger` free; no
   SkyrimSE / ModOrganizer process (Get-Process, checked right before).
2. Install into a NEW mod folder of its own (never the posted "World Map Selector" folder):
   `python "D:\Claude output\.MD\scripts\install-test-build.py" "<package folder above>" "test World Map Selector for Controller 1.0.0"`
3. Enable it in the "Njordlinger Test" profile and DISABLE the original "World Map Selector" (rule 40) - with both
   loaded, ours logs an error and stays inactive by design. Back up `modlist.txt` first, restore it afterwards.
4. Steam signed in (controller tests read nothing without Steam Input). If the owner's controller is not in use and
   he has not said otherwise, the ViGEm virtual pad (`.MD\scripts\vpad\vpad.py`) may stand in for it; unplug it the
   moment the step is done (past-mistakes library, 2026-09-28).
5. Launch only through `game-relaunch.ps1 -Profile "Njordlinger Test"`; New Game through Alternate Perspective, `coc`
   somewhere in Skyrim (e.g. `coc Whiterun`).

## What a good result looks like

| # | Action | Expect |
|---|---|---|
| 0 | Read the log `Documents\My Games\Skyrim Special Edition\SKSE\WorldMapSelectorForController.log` | `World Map Selector for Controller 1.0.0 loaded successfully`; `Registered the world-map chooser trigger: key 0x2E, controller button 0x0001 (MapMenu only)`; `Text language: english (12 ...)`; `Registered "wmsfc.control"`; NO line about the original DLL |
| 1 | In gameplay (no menu), press D-pad up, then C | nothing happens from this mod (vanilla: D-pad up = Favourites / the owner's Toggle POV; C does nothing); `wmsfc.control` shows `ignoredOutsideMap` grown, `chooserActive:false` |
| 2 | Open the world map (controller), press D-pad up | the "Select World Map (1/N)" message box opens over the map; `opensFromGamepad` = 1, `chooserActive:true` |
| 3 | Pick Solstheim (needs Dragonborn) | the map closes and reopens on Solstheim; HUD "World map selected: Solstheim" |
| 4 | Open the chooser again, press B / the map's Cancel | the chooser closes, the map stays |
| 5 | With the map open, use the keyboard: press C | the chooser opens; `opensFromKeyboard` = 1 |
| 6 | SkyUI button bar on the world map | a `Select Map` hint: D-pad-up art after controller input, C after keyboard input |
| 7 | `[Clear Selection]` | HUD "World map selection cleared."; next map open is the world you are in |

Driving without hands: TestBench `POST /api/tool/wmsfc.control` with `{}` (state), `{"op":"open"}` (asks through the
same MapMenu-only gate - expect nothing outside the map), `{"op":"dismiss"}`. The real proof for items 1, 2 and 5 is
the real input path - a virtual-pad press and a spliced keyboard press - not `op=open`.

Evidence per PREFLIGHT: `gamewatch.ps1 snap wmsfc-chooser-open` with the chooser up, the log lines above, the tool
JSON before/after. Then `version-gate.ps1 -Action record -Mod WorldMapSelectorForController -Version 1.0.0 -Status
working -Evidence "<what was seen>" -Dll "<installed DLL path>"`. Close the game gracefully (TestBench `qqq`, then
CloseMainWindow), release the lock, restore the profile files.
