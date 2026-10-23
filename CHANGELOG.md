# WorldMapSelectorForController - changelog

Written as changes happen, not reconstructed afterwards (rule 61). Each version carries its
**version-ledger status**, so this file cannot quietly claim more than the ledger does:

* **working** - observed running in game
* **untested** - built and packaged, not yet confirmed
* **failed** - built but crashed or malfunctioned; the number was reclaimed
* **scratch** - a hypothesis-test build that never held a real number

## 1.0.0 - 2026-10-23 - untested

### Added
- A DevBench/TestBench tool, wmsfc.control, that reports the chooser's live state and can open or close it through the same world-map-only check a press uses.
- The chooser's text in eleven languages (English, Japanese, Korean, Chinese, Russian, German, French, Spanish, Italian, Polish, Czech), following the game's language.
- SkyUI's button bar on the world map shows the chooser's button while a controller is in use and its key otherwise.
- The map chooser opens from inside the world map with one button: D-pad up on a controller, C on the keyboard (MapMenuButton / MapMenuKey in [Controls]). Outside the world map both do nothing.

### Changed
- Installs as WorldMapSelectorForController.dll with its own WorldMapSelectorForController.ini. If the original WorldMapSelector.dll is also installed, this plugin logs an error and stays inactive rather than hooking the same game code twice.
### Removed
- The global hotkey (OpenSelectorKey, F10) and the settings that only applied to it: OpenMapAfterSelection, AllowChooserOutsideMap and AllowChooserWhileMapOpen. An INI that still carries them gets a warning line in the log and is otherwise unaffected.