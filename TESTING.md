# World Map Selector for Controller - in-game test steps (1.0.0, map list panel)

Written 2026-10-23. Build under test: the 1.0.0 package with the vertical map list panel and the settings page
(commit 0e1fb60 and later). Run through Main Agent in the combined round (rule 69) when `instance:Njordlinger` is
free and no SkyrimSE / ModOrganizer process runs.

## Build under test

- Package: `D:\Claude output\7. current test builds\Skyrim - World Map Selector for Controller\World Map Selector for Controller 1.0.0`
- Packaged DLL sha256 starts `5A16D3FC`; check the installed DLL's hash matches before judging.
- Needs the Apocrypha Menu Framework enabled (the profile's "Apocrypha Menu Framework" mod, 2.2.x or newer). Without it
  the plugin falls back to the old message box - that is a fallback, not this test.

## Setup

1. `resolve.py run health-gate` passes; `agent-lock status` shows `instance:Njordlinger` free (it shares a session id
   with sibling sub-agents - read the holder NAME, do not rely on the tool refusing); Get-Process shows no SkyrimSE /
   ModOrganizer right before installing.
2. Install: `python "D:\Claude output\.MD\scripts\install-test-build.py" "<package folder above>" "test World Map Selector for Controller 1.0.0"`
   (keeps the folder's INI; the new `[Panel]` keys take their compiled defaults - Right, 22, 100 - when absent).
3. Back up `profiles\Njordlinger Test\modlist.txt`; enable `test World Map Selector for Controller 1.0.0`, DISABLE the
   original `World Map Selector` (rule 40). Restore modlist.txt afterwards.
4. Launch only through `game-relaunch.ps1 -Profile "Njordlinger Test"`; New Game through Alternate Perspective,
   `coc Whiterun` (or any exterior).

## What a good result looks like

| # | Action | Expect |
|---|---|---|
| 0 | Log `Documents\My Games\Skyrim Special Edition\SKSE\WorldMapSelectorForController.log` | `1.0.0 loaded successfully`; `Loaded configuration: ... Panel Side=Right, WidthPercent=22, TextScalePercent=100`; `Built map catalog with 6 selectable worldspaces` (this list: Skyrim, Solstheim, Soul Cairn, Shivering Isles, Tamriel/Beyond Skyrim, Wyrmstooth); `Registered the map list panel with the menu framework (side right, width 22%, text 100%)`; `Registered the settings page with the menu framework.` AMF's log: `AddWindow (SMF-compat): consumer window registered` |
| 1 | Open the world map (controller), press D-pad up | a vertical panel down the RIGHT side of the map, top to bottom of the map area (8% margin top, 12% bottom), titled "World Maps", Nordic theme frame around it, `[Clear Selection]` first, then `Skyrim [Here]`, then the others alphabetically; highlight on `Skyrim [Here]`. AMF log: `input: a mod's window "##WMSFCMapPanel" took the input`. Map behind it does NOT pan when the stick moves |
| 2 | D-pad down / left stick down several times | highlight moves one row per press; map does not move |
| 3 | `wmsfc.control {"op":"fake","count":30}`, B to close, D-pad up to reopen | 36+ rows; press down past the bottom - the list scrolls to keep the highlight in view (`panel.scrollY` grows, `scrollMaxY` > 0) |
| 4 | Highlight Solstheim, press A | panel closes; map closes and reopens on Solstheim; HUD "World map selected: Solstheim" |
| 5 | Reopen, highlight `[Clear Selection]`, A | HUD "World map selection cleared."; the map stays open; next map open shows the world you are in |
| 6 | Reopen, press B | panel closes, the map stays open, and nothing else happens on the map (no menu backs out) |
| 7 | AMF menu -> World Map Selector for Controller -> Settings: Side = Left | close AMF, open the panel: it runs down the LEFT side. The log shows `Saved settings: Side=Left, ...`; the mod folder's INI has `[Panel] Side=Left` |
| 8 | Width and Text size sliders: D-pad left/right | one unit (1%) per nudge (rule 68); the panel follows live |
| 9 | Restart the game (game-relaunch.ps1) | `Loaded configuration: ... Panel Side=Left` - the setting persisted; set it back to Right afterwards (or Restore defaults) |
| 10 | Keyboard/mouse: C opens; arrows / wheel move; Enter or a click chooses; Esc closes | as 1-6 |

Driving without hands: TestBench `POST /api/tool/wmsfc.control` - `{}` state (the `panel` object: open, rows,
highlight, scrollY, scrollMaxY, rect, side, widthPercent, textScalePercent), `{"op":"open"}`, `{"op":"dismiss"}`,
`{"op":"nav","rows":N}`, `{"op":"select"}`, `{"op":"fake","count":N}` (test only; 0 removes), `{"op":"settings",
"side":"left","width":25,"text":110}`. The real proof for items 1, 2, 4, 6 and 10 is the real input path (a
virtual-pad press, a spliced key), not the ops. Remove the fake rows (`count:0`) before taking screenshots that
represent the shipped list.

Evidence: `gamewatch.ps1 snap` (or `amf.process op=capture`; desktop-grab if black) with the panel up on the right,
scrolled with fake rows, after Side=Left; the log lines; tool JSON before/after. Then
`version-gate.ps1 -Action record -Mod WorldMapSelectorForController -Version 1.0.0 -Status working -Evidence "<seen>"
-Dll "<installed DLL>"`. Close the game gracefully (TestBench `qqq`, then CloseMainWindow), delete only this run's
save, restore modlist.txt and the INI's `[Panel]` values, release the lock.

## Which worldspaces are listed (checked 2026-10-23)

Discovery is World Map Selector 1.4.0's, unchanged (diffed identical): a worldspace is listed when it owns its map
data (it does not take the parent's map through the "use parent map data" flag) and its map cell bounds are not
degenerate. A scan of every WRLD record in the Njordlinger Test load order (1834 plugins) finds exactly six such
worldspaces: Tamriel (Skyrim), DLC2SolstheimWorld (Solstheim), DLC01SoulCairn (Soul Cairn), ECSSShiveringIsles
(Shivering Isles), BSHeartland (Tamriel, Beyond Skyrim) and WyrmstoothWorld (Wyrmstooth) - the six the 1.0.0
message box showed. Dayspring Canyon (DLC1HunterHQWorld), Forgotten Vale (DLC01FalmerValley), Blackreach,
Sovngarde, Skuldafn, the city worlds and the other caves use their parent's map (Skyrim's; Apocrypha uses
Solstheim's, Boneyard Soul Cairn's, Dimfrost Wyrmstooth's), so they have no map of their own to select - choosing
them would show the same map as the parent.
