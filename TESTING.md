# World Map Selector for Controller - in-game test steps (1.0.0, map list panel, panel style, presses kept)

Written 2026-10-23. Build under test: the 1.0.0 package with the vertical map list panel and the settings page
(commit 0e1fb60 and later). Run through Main Agent in the combined round (rule 69) when `instance:Njordlinger` is
free and no SkyrimSE / ModOrganizer process runs.

## Build under test

- Package: `D:\Claude output\7. current test builds\Skyrim - World Map Selector for Controller\World Map Selector for Controller 1.0.0`
- Packaged DLL sha256 starts `863BF0DE` (the 2026-10-23 every-switch-immediate + claim-end rebuild); check the installed DLL's hash matches before judging.
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
| 5 | Reopen, highlight `[Clear Selection]`, A | HUD "World map selection cleared."; the map closes and reopens on the world you are in at once (corrected 2026-10-23 - see 18-23) |
| 6 | Reopen, press B | panel closes, the map stays open, and nothing else happens on the map (no menu backs out) |
| 7 | AMF menu -> World Map Selector for Controller -> Settings: Side = Left | close AMF, open the panel: it runs down the LEFT side. The log shows `Saved settings: Side=Left, ...`; the mod folder's INI has `[Panel] Side=Left` |
| 8 | Width and Text size sliders: D-pad left/right | one unit (1%) per nudge (rule 68); the panel follows live |
| 9 | Restart the game (game-relaunch.ps1) | `Loaded configuration: ... Panel Side=Left` - the setting persisted; set it back to Right afterwards (or Restore defaults) |
| 10 | Keyboard/mouse: C opens; arrows / wheel move; Enter or a click chooses; Esc closes | as 1-6 |

### Added 2026-10-23 (same 1.0.0): Norden UI - Black panel style, and presses kept from other mods

Install also Norden UI - Black 1.1.0 (`7. current test builds\Skyrim - Norden UI - Black\Norden UI - Black 1.1.0 FOMOD`)
with its "World Map Selector for Controller" option (auto-ticked when the WMS DLL is installed), so the mod folder holds
`SKSE\Plugins\WorldMapSelectorForController_PanelStyle.ini`. Simplest Horses' horse call stays on D-pad up (the owner's
`MCM\Settings\SimplestHorses.ini` `iHorse_hotkey = 266`, UVC's "Call Horse" row).

| # | Action | Expect |
|---|---|---|
| 11 | Log at start | `Panel style "Norden UI - Black" read from ...WorldMapSelectorForController_PanelStyle.ini (theme frame off, line thickness 1.0)`; no `does not export` warning |
| 12 | World map, D-pad up | the panel is black and opaque, white row text, the highlighted row has Norden's item-list selection (a faint slate band, light lines along its top and bottom fading to the right, a thin mark at its left), slate edge lines along the panel's top and bottom fading at both ends, no Nordic knotwork frame; screenshot it. `wmsfc.control {}`: `panel.style.active=true`, `name="Norden UI - Black"` |
| 13 | Side, Width and Text size on the settings page (as 7-8) | still apply live with the style on |
| 14 | Same press (D-pad up) in the world map with Simplest Horses on D-pad up | the chooser opens and the horse is NOT called (no whistle, no horse running up) - press, hold a second, release, B to close; WMS log (Debug) `the panel opened and the press is kept from the game and other mods` and `release of device 2 code 0x1 kept from the game`; `wmsfc.control {}` `panel.claimedHeld` back to 0 after the release |
| 15 | Close the map, D-pad up in gameplay | the horse IS called (Simplest Horses works as before); WMS log `pressed outside MapMenu; passed on to the game` |
| 16 | B, Esc, A, Enter and D-pad down while the panel is open | none of them reaches another mod or the map (the map does not back out on B; nothing bound to those buttons by another mod fires) |
| 17 | Untick the option / delete `WorldMapSelectorForController_PanelStyle.ini`, restart | log `No panel style file`; the panel looks as 1.0.0 (theme colours and frame) |
| 17a | (2026-10-23, after batch 3) With the style on, look at the two separators (under "World Maps", above the hint) | each is ONE solid slate (#5B676D) line, 1 px, fading out at both ends - NOT the theme's dotted rule with end dots. Batch 3 (build 9AAE5C21) showed the dotted rule: AMF draws ImGui's Separator as its theme's section art, so the style now draws its own line |
| 17b | Same frame: the panel's top and bottom edges | a slate (#5B676D) line along each, solid in the middle and fading at both ends, no knotwork frame. Judge it from an in-process capture (`amf.process op=capture`) or a pixel read: batch 3's desktop grab was about 2.1x too bright (slate read as 193,219,231), so a thin slate line on the bright map looked absent. It was present (rows 16 and 879 of the batch-3 crop) |
| 17c | Same frame: the footer hint | dim grey (#999999), visibly darker than the white rows. In the 2.1x-bright desktop grab both clip to white; compare in an in-process capture (or by pixel: rows white 255,255,255; hint 153,153,153) |

### Added 2026-10-23 (same 1.0.0): every switch is immediate, in one map visit

The owner (batch 3): the first switch worked, but going back to the previous map or another one needed the map closed
and reopened. Cause: the panel judged "already showing" against the world the player is in, not the map on screen (live
log 05:35:28 `chose Skyrim, the map already showing` while the map showed Tamriel/BSHeartland). Do 18-23 in ONE world
map visit, never closing the map yourself, standing in Skyrim (A = Skyrim, B = Solstheim, C = Shivering Isles or any
third map). Screenshot after each switch.

| # | Action | Expect |
|---|---|---|
| 18 | Open the map, D-pad up | panel highlight on `Skyrim [Here]` (or `[Here/Selected]`) |
| 19 | Choose Solstheim (A -> B) | map closes and reopens on Solstheim at once; log `Map list panel selected Solstheim`, `closing MapMenu before reopening it`, `MapMenu worldspace: name="Solstheim"` |
| 20 | D-pad up, choose Skyrim (B -> A) | the panel opened with the highlight on `Solstheim [Selected]`; the map reopens on Skyrim at once; log `Map list panel selected Skyrim (0000003C)`, `closing MapMenu before reopening it`, `Map selection: Default (requested map 0000003C is current)`, `MapMenu worldspace: name="Skyrim"`; NO `the map already showing` line |
| 21 | D-pad up, choose Shivering Isles (A -> C) | highlight was on `Skyrim [Here/Selected]`; the map reopens on Shivering Isles at once; `MapMenu worldspace: name="Shivering Isles"` |
| 22 | D-pad up, `[Clear Selection]`, A | highlight was on `Shivering Isles [Selected]`; HUD "World map selection cleared."; the map reopens on Skyrim at once; log `cleared the map selection; switching back to the player's map`, `Map selection: Default.`, `MapMenu worldspace: name="Skyrim"` |
| 23 | D-pad up, choose Skyrim again (already showing) | the panel highlight is on `Skyrim [Here]`; log `chose Skyrim, the map already showing`; the map does NOT close and reopen |
| 24 | Right after a switch (A held a moment on the choice), close the map and press A once on a door or NPC; then reopen the map, D-pad up, B to close the panel, close the map, press A again | A works in game on the FIRST press both times (activates). If a release went missing the log shows `pressed again before its release was seen; the old claim is dropped and this press is not kept`; `wmsfc.control {}` `panel.claimedHeld` is 0 once nothing is held. A press is never eaten in game |

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

## Result, 2026-10-23 (WMS Panel Agent) - recorded working

Two runs on Njordlinger Test. Run 1 (build 5A16D3FC): items 0-7 and the 30-test-row scroll (scrollY 0 -> 330 =
scrollMaxY, highlight kept in view) passed; one defect found - the first controller press after keyboard or mouse use
was lost, because the panel read its buttons from ImGui and the framework spends that press switching its input mode.
Fixed by reading the panel's keys and buttons through the framework's input callback (RegisterInpoutEvent), which
sees every event first; the footer hint also wraps now and follows the device used. Run 2 (build 16C4E49F, the
packaged one): Side=Left persisted across the restart; pad open, D-pad and stick one row per press, A chose Soul
Cairn (map switched), B closed on the first press after keyboard use, Esc closed, C opened, a mouse click on
[Clear Selection] cleared, Restore defaults wrote Side=Right. Screenshots and logs: `build/test-evidence-1.0.0-panel/`.
Not exercised: the width / text-size sliders' one-unit nudge (shared precise::SliderInt wrapper).
