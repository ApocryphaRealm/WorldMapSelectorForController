World Map Selector for Controller
Version 1.0.0

View and fast travel from world maps other than the one you are in. Open the world map and press
D-pad up on a controller, or C on the keyboard, to open the list of maps down the right side of
the map (scroll with the D-pad, stick or wheel; A or Enter chooses; B or Esc closes). Outside the
world map neither press does anything. In the world map the chooser keeps its presses to itself:
the press that opens it and every key or button it acts on never reach another mod's hotkey or a
script's key event (needs the Apocrypha Menu Framework).

Panel style: a UI mod can ship SKSE\Plugins\WorldMapSelectorForController_PanelStyle.ini to give
the panel its own colours, highlight and edge lines (Norden UI - Black does, as an installer
option). Without it the panel wears the Apocrypha Menu Framework's theme. See the mod's README for
the keys.

Settings: SKSE\Plugins\WorldMapSelectorForController.ini ([Controls] MapMenuButton / MapMenuKey).
Requires SKSE64 and Address Library for SKSE Plugins. The side panel and the settings page need the
Apocrypha Menu Framework; without it the maps are offered in a message box. SkyUI is optional
(button-bar hint only).
Do not install beside the original World Map Selector (WorldMapSelector.dll).

Log: Documents\My Games\Skyrim Special Edition\SKSE\WorldMapSelectorForController.log

Built on World Map Selector by Apocrypher00 (Nexus 187315), released under the Unlicense.
The DLL links CommonLibSSE NG and is distributed under GPL-3.0-or-later with its exceptions;
licence texts: SKSE\Plugins\WorldMapSelectorForController\Licenses\
