# Third-Party Notices

WorldMapSelector-authored source is released under the Unlicense. The combined
plugin binary is governed by GPL-3.0-or-later with CommonLibSSE NG's exceptions.
See `LICENSING.md` for that distinction. Neither grant replaces the licenses of
other third-party projects used to build or run WorldMapSelector.

## Runtime requirements (not redistributed)

| Project | Purpose |
| --- | --- |
| [SKSE64](https://skse.silverlock.org/) | Loads and provides runtime services to the plugin. |
| [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444) | Resolves version-specific Skyrim executable addresses. |

## Linked and build dependencies

| Project | License | Copyright / attribution |
| --- | --- | --- |
| [CommonLibSSE NG](https://github.com/alandtse/CommonLibSSE-NG) | GPL-3.0-or-later with Modding and Linking Exceptions | Copyright (c) 2018 Ryan-rsm-McKenzie and subsequent contributors. Includes work from CommonLibSSE and CommonLibSSE-NG. |
| [MinHook](https://github.com/TsudaKageyu/minhook) | 2-clause BSD | Copyright (c) 2009-2017 Tsuda Kageyu. Includes Hacker Disassembler Engine portions by Vyacheslav Patkov. |
| [fmt](https://github.com/fmtlib/fmt) | MIT | Copyright (c) 2012-present Victor Zverovich and fmt contributors. |
| [spdlog](https://github.com/gabime/spdlog) | MIT | Copyright (c) 2016 Gabi Melman. |
| [Xbyak](https://github.com/herumi/xbyak) | 3-clause BSD | Copyright (c) 2007 MITSUNARI Shigeo. |
| [rapidcsv](https://github.com/d99kris/rapidcsv) | 3-clause BSD | Copyright (c) 2017 Kristofer Berggren. |
| [SimpleIni](https://github.com/brofield/simpleini) | MIT | Copyright (c) 2006-2024 Brodie Thiesfield. |
| [toml11](https://github.com/ToruNiina/toml11) | MIT | Copyright (c) 2017 Toru Niina. |
| [JSON for Modern C++](https://github.com/nlohmann/json) | MIT | Copyright (c) 2013-2025 Niels Lohmann. |
| [DirectXMath](https://github.com/microsoft/DirectXMath) | MIT | Copyright (c) Microsoft Corporation. |
| [DirectX Tool Kit](https://github.com/microsoft/DirectXTK) | MIT | Copyright (c) Microsoft Corporation. |
| [devbench](https://github.com/alandtse/devbench) consumer API (`src/DevBench/DevBenchAPI.h`, `.cpp`) | MIT (separate from devbench's own GPL-3.0) | Copyright (c) 2026 Alan Tse and devbench contributors. Notice: `src/DevBench/DevBenchAPI.LICENSE.txt`. |

The exact dependency revisions used by a build are pinned by `vcpkg.json`, its
builtin baseline, and the CommonLibSSE NG Git submodule commit.

## License terms

The complete GPL text and exceptions for CommonLibSSE NG are present in its
checked-in Git submodule at `extern/CommonLibVR/COPYING` and
`extern/CommonLibVR/EXCEPTIONS.md`. Exact license texts for vcpkg
dependencies are installed under each package's `share/<package>/copyright`
path during the build.

Binary distributions must be accompanied by the applicable copyright and
license texts for linked third-party components. In particular, the MinHook
and BSD-licensed components require their notices and disclaimers to accompany
binary redistributions.

For a public mod archive, consolidate those required texts into a
WorldMapSelector-specific notice file beneath
`SKSE/Plugins/WorldMapSelectorForController/`. Do not place generic files such as
`LICENSE`, `COPYING`, or `README` at the archive root: mod managers treat the
archive as a `Data`-directory payload, and generic names can collide with files
from unrelated mods. This repository's summary is not a replacement for the
complete license texts that must accompany the binary release.

Build tools such as CMake, Ninja, vcpkg, Git, and Microsoft Visual C++ are not
redistributed as part of WorldMapSelector.
