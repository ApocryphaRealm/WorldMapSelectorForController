# WorldMapSelectorForController - changelog

Written as changes happen, not reconstructed afterwards (rule 61). Each version carries its
**version-ledger status**, so this file cannot quietly claim more than the ledger does:

* **working** - observed running in game
* **untested** - built and packaged, not yet confirmed
* **failed** - built but crashed or malfunctioned; the number was reclaimed
* **scratch** - a hypothesis-test build that never held a real number

## 1.1.0 - 2026-11-05 - untested

### Added
- A switch for each world found, on the settings page under Maps in the list. Off hides that land from the chooser: it can't be chosen, and if it was the chosen map the choice goes back to the world you are in. The switches write `[Compatibility] ExcludedWorldspaces`, the list the INI already had; without the menu framework that line is the control. In the eleven languages.
- `[WorldspaceNames]` in the INI: a world's name in the chooser, as EditorID=name, over its record's name. Ships `BSHeartland=Bruma`. An INI kept from an older version gets the same name without the section.
- wmsfc.control: the state lists every world found under `worlds`, and op=hide is a world's switch.

### Fixed
- Beyond Skyrim's Bruma is called Bruma in the chooser, not Tamriel (its record's name).
- Inside a child world that owns its map (Dayspring Canyon and the others World Map Render gives a map), the world map opens on that world, with the player's mark, and the list tags it [Here]; it opened on Skyrim's before, because the game answers with the parent's map whatever the flag says. A map chosen in the list still wins, and choosing Skyrim from inside such a world shows Skyrim's markers, not that world's. Cities and other worlds that keep the Use Map Data flag, worlds with no map bounds, and interiors keep the game's answer, so nothing changes without World Map Render. [Clear Selection] goes back to that world's map.
- The list of maps no longer depends on which DLL handles the game's data-loaded message first. It is read again each time the chooser opens, and once more after data load for the settings page, so a land World Map Render gives its own map (Blackreach, Dayspring Canyon, Forgotten Vale, Sovngarde, Apocrypha, Skuldafn) is listed. A world that uses its parent's map stays out when nothing gives it one.
- The chooser's trigger no longer opens the map list panel while another menu framework window that blocks input is open, such as Map Marker List's panel; the press is passed on to that window.

## 1.0.1 - 2026-11-03 - untested

### Fixed
- The keyboard key that opens the chooser (C by default) can no longer reach the game's Auto-Move from the world map. C is Auto-Move in vanilla gameplay; a C pressed in the map while the panel was already open, or while a message box was up, was passed on, so if it was still held as the map closed its repeats and release landed in gameplay. Every press of the chooser's key in the world map is now kept from the game until it is let go, even after the map has closed. Players with Unbind Vanilla Controls (where C is unbound) see no change. Needs the Apocrypha Menu Framework, as the other kept presses do.
- The settings page's sliders clamp a typed value (Ctrl+click) to their range inside the menu framework and again afterwards, so a number typed past the end of the width or text-size slider cannot be kept or re-applied every frame.

- **Nothing told a player the mod had stood down.** With the original World Map Selector also on, this mod stays inactive (both hook the same game code) and only its log said so: D-pad up on the map did nothing and the panel never opened (the owner's report, 2026-11-03; his log's fifth and last line was that error, while the original logged every map). It now says so in a notification on every game load, in the eleven languages, until one of the two is off.

### Changed
- MapMenuKey and MapMenuButton are read again each time the world map opens, so a key set while the game runs applies at the next map. Unbind Vanilla Controls 1.3.0 lists them as its Map Selector row, in its IN MAP block.
- The description no longer says C "does nothing" outside the world map: outside the map the mod leaves the key and button to the game, so C keeps its vanilla Auto-Move there and D-pad up whatever your layout gives it.

## 1.0.0 - 2026-10-23 - working

### Added
- A panel style file: another mod can ship SKSE\Plugins\WorldMapSelectorForController_PanelStyle.ini ([PanelStyle]: sBackground, sText, sTextHighlighted, sTextDim, sLine, sScrollbar, sHighlightBand, sHighlightLine, sHighlightMark, bThemeFrame, fLineThickness; colours #RRGGBB or #RRGGBBAA, every key optional) to give the map list panel its own colours, row highlight and edge lines. Norden UI - Black 1.1.0 ships one in its "World Map Selector for Controller" option. Without the file the panel is drawn exactly as before. The DevBench state reports the style in use.
- Presses the chooser takes are kept from the game and every other mod: in the world map, the trigger (D-pad up / C) that opens the panel, every key and button the panel acts on while it is open, and the release of each of them (even after the panel has closed) are taken out of the game's input inside the menu framework's input hook, before any other plugin's input sink or a Papyrus OnKeyDown / OnKeyUp sees them. Opening the chooser no longer also fires another mod's hotkey on the same button (Simplest Horses' horse call on D-pad up, which acts on the release). Outside the world map the trigger is left to the game. Without the Apocrypha Menu Framework (the message-box chooser) presses are not taken.
- wmsfc.control drives the panel: its rows, highlight, scroll and rectangle in the state; op=nav, op=select, op=settings, and a test-only op=fake that adds rows to prove the list scrolls.
- A settings page in the Apocrypha Menu Framework: the panel's side (Right or Left), width and text size, and the switches for keeping the chosen map, the [Clear Selection] row and the button hints. Changes apply at once and are saved to the INI ([Panel] Side, WidthPercent, TextScalePercent).
- The maps are listed in a vertical panel down one side of the world map (right by default), as tall as the map and scrollable, so worldspaces added by other mods all fit. D-pad or left stick moves the highlight, A chooses, B closes and leaves the map open; on PC the arrow keys or mouse wheel move, Enter or a click chooses, Esc closes. The panel uses the Apocrypha Menu Framework's theme and frame; without the framework the message-box chooser is used.
- A DevBench/TestBench tool, wmsfc.control, that reports the chooser's live state and can open or close it through the same world-map-only check a press uses.
- The chooser's text in eleven languages (English, Japanese, Korean, Chinese, Russian, German, French, Spanish, Italian, Polish, Czech), following the game's language.
- SkyUI's button bar on the world map shows the chooser's button while a controller is in use and its key otherwise.
- The map chooser opens from inside the world map with one button: D-pad up on a controller, C on the keyboard (MapMenuButton / MapMenuKey in [Controls]). Outside the world map both do nothing.

### Fixed
- With a panel style file, the panel's two separators (under the title and above the hint) are the style's own line (sLine, fading at both ends) instead of the menu framework theme's dotted section rule, which the framework draws for every ImGui separator. Without a style file they are unchanged.
- Every choice in the world map now switches the map at once, however many times it is used in one visit: going back to the map of the world you are in while another map is shown, and [Clear Selection] while another map is shown, now close and reopen the map on your own world straight away, instead of only taking effect the next time the map was opened. The panel had taken the world you are in for the map on screen, so it treated that choice as "already showing" and did nothing. The message-box chooser's [Clear Selection] does the same.
- A button the panel took is no longer able to eat a later press. The panel keeps each press it acts on (and its release) from the game and other mods until the release; if that release went missing - it can while a map switch closes and reopens the map - the button stayed marked as held, and the next press of it anywhere in the game (A: activate, accept) was swallowed. A new press of a marked button now ends the old mark and goes through. The panel's short wait after a choice also ends as soon as A or B is let go.

### Changed
- The message-box chooser (Select World Map (1/N), paged left and right) is now the fallback for players without the Apocrypha Menu Framework; MapsPerPage and ShowCancelButton apply to it only. Which worldspaces are offered is unchanged from World Map Selector 1.4.0: every worldspace with map data of its own.
- Installs as WorldMapSelectorForController.dll with its own WorldMapSelectorForController.ini. If the original WorldMapSelector.dll is also installed, this plugin logs an error and stays inactive rather than hooking the same game code twice.
### Removed
- The global hotkey (OpenSelectorKey, F10) and the settings that only applied to it: OpenMapAfterSelection, AllowChooserOutsideMap and AllowChooserWhileMapOpen. An INI that still carries them gets a warning line in the log and is otherwise unaffected.