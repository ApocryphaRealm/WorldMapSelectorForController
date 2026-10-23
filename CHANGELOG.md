# WorldMapSelectorForController - changelog

Written as changes happen, not reconstructed afterwards (rule 61). Each version carries its
**version-ledger status**, so this file cannot quietly claim more than the ledger does:

* **working** - observed running in game
* **untested** - built and packaged, not yet confirmed
* **failed** - built but crashed or malfunctioned; the number was reclaimed
* **scratch** - a hypothesis-test build that never held a real number

## 1.0.0 - 2026-10-23 - untested

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
- Every choice in the world map now switches the map at once, however many times it is used in one visit: going back to the map of the world you are in while another map is shown, and [Clear Selection] while another map is shown, now close and reopen the map on your own world straight away, instead of only taking effect the next time the map was opened. The panel had taken the world you are in for the map on screen, so it treated that choice as "already showing" and did nothing. The message-box chooser's [Clear Selection] does the same.

### Changed
- The message-box chooser (Select World Map (1/N), paged left and right) is now the fallback for players without the Apocrypha Menu Framework; MapsPerPage and ShowCancelButton apply to it only. Which worldspaces are offered is unchanged from World Map Selector 1.4.0: every worldspace with map data of its own.
- Installs as WorldMapSelectorForController.dll with its own WorldMapSelectorForController.ini. If the original WorldMapSelector.dll is also installed, this plugin logs an error and stays inactive rather than hooking the same game code twice.
### Removed
- The global hotkey (OpenSelectorKey, F10) and the settings that only applied to it: OpenMapAfterSelection, AllowChooserOutsideMap and AllowChooserWhileMapOpen. An INI that still carries them gets a warning line in the log and is otherwise unaffected.