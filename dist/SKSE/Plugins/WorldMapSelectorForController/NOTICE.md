# Notice

World Map Selector for Controller is built on **World Map Selector** by **Apocrypher00**:

- Nexus Mods: https://www.nexusmods.com/skyrimspecialedition/mods/187315
- Source: https://github.com/Apocrypher00/WorldMapSelector (forked at tag v1.4.0, commit a752851)
- Licence of that source: the Unlicense (public domain dedication), kept here as `UNLICENSE`.
  The author's Nexus permissions read: the independently authored source "is released under the
  Unlicense. You may copy, modify, publish, use, compile, sell, or distribute that source without
  restriction", and distributed binaries remain subject to CommonLibSSE NG's GPL-3.0-or-later terms.

The Unlicense asks for no attribution; it is given here because the world-map switching, marker and
fast-travel work is that author's.

## The combined binary

`WorldMapSelectorForController.dll` links CommonLibSSE NG 6.7.0 (submodule `extern/CommonLibVR`, commit
3d81614617910e7f34b33d8750881811b5e36445) statically. CommonLibSSE NG is GPL-3.0-or-later with its Modding
Exception and GPL-3.0 Linking Exception, so the distributed DLL is governed by **GPL-3.0-or-later** with those
exceptions; its corresponding source is this repository at the release's tag plus that submodule commit and
the `vcpkg.json` baseline. The full texts ship in the package under
`SKSE/Plugins/WorldMapSelectorForController/Licenses/`.

## Our changes (2026-10-23 on)

- The chooser opens from inside the world map with one button (controller D-pad up, keyboard C); the
  global hotkey and its outside-the-map options are removed.
- The SkyUI hint follows the device in use; the chooser's text is translated into eleven languages.
- A DevBench/TestBench tool (`wmsfc.control`); the vendored DevBench consumer API (`src/DevBench/`) is MIT,
  its notice kept beside it as `DevBenchAPI.LICENSE.txt`.
- Renamed DLL/INI, and a stand-down when the original DLL is also loaded.

Other third-party components keep their own licences: see `THIRD_PARTY_NOTICES.md`.
