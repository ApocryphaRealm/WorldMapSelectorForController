# WorldMapSelectorForController - changelog

Written as changes happen, not reconstructed afterwards (rule 61). Each version carries its
**version-ledger status**, so this file cannot quietly claim more than the ledger does:

* **working** - observed running in game
* **untested** - built and packaged, not yet confirmed
* **failed** - built but crashed or malfunctioned; the number was reclaimed
* **scratch** - a hypothesis-test build that never held a real number

## 1.0.0 - 2026-10-23 - working

### Added
- wmsfc.control drives the panel: its rows, highlight, scroll and rectangle in the state; op=nav, op=select, op=settings, and a test-only op=fake that adds rows to prove the list scrolls.
- A settings page in the Apocrypha Menu Framework: the panel's side (Right or Left), width and text size, and the switches for keeping the chosen map, the [Clear Selection] row and the button hints. Changes apply at once and are saved to the INI ([Panel] Side, WidthPercent, TextScalePercent).
- The maps are listed in a vertical panel down one side of the world map (right by default), as tall as the map and scrollable, so worldspaces added by other mods all fit. D-pad or left stick moves the highlight, A chooses, B closes and leaves the map open; on PC the arrow keys or mouse wheel move, Enter or a click chooses, Esc closes. The panel uses the Apocrypha Menu Framework's theme and frame; without the framework the message-box chooser is used.
- A DevBench/TestBench tool, wmsfc.control, that reports the chooser's live state and can open or close it through the same world-map-only check a press uses.
- The chooser's text in eleven languages (English, Japanese, Korean, Chinese, Russian, German, French, Spanish, Italian, Polish, Czech), following the game's language.
- SkyUI's button bar on the world map shows the chooser's button while a controller is in use and its key otherwise.
- The map chooser opens from inside the world map with one button: D-pad up on a controller, C on the keyboard (MapMenuButton / MapMenuKey in [Controls]). Outside the world map both do nothing.

### Changed
- The message-box chooser (Select World Map (1/N), paged left and right) is now the fallback for players without the Apocrypha Menu Framework; MapsPerPage and ShowCancelButton apply to it only. Which worldspaces are offered is unchanged from World Map Selector 1.4.0: every worldspace with map data of its own.
- Installs as WorldMapSelectorForController.dll with its own WorldMapSelectorForController.ini. If the original WorldMapSelector.dll is also installed, this plugin logs an error and stays inactive rather than hooking the same game code twice.
### Removed
- The global hotkey (OpenSelectorKey, F10) and the settings that only applied to it: OpenMapAfterSelection, AllowChooserOutsideMap and AllowChooserWhileMapOpen. An INI that still carries them gets a warning line in the log and is otherwise unaffected.