# Licensing

WorldMapSelector distinguishes between its independently authored source code and
the combined plugin binary produced by the build.

## WorldMapSelector source

Source code and documentation authored specifically for WorldMapSelector are
dedicated to the public domain under the [Unlicense](UNLICENSE), except where a file
or incorporated third-party component states otherwise. Anyone may copy, modify,
publish, distribute, sublicense, sell, or otherwise reuse that WorldMapSelector-authored
material without an attribution or source-publication requirement.

## Combined plugin binary

WorldMapSelector links CommonLibSSE NG statically into `WorldMapSelectorForController.dll`.
CommonLibSSE NG is licensed under GPL-3.0-or-later with its Modding Exception and
GPL-3.0 Linking Exception (with Corresponding Source). Distribution of the combined
DLL is therefore governed by those terms, including their corresponding-source and
notice requirements.

The complete CommonLibSSE NG terms are preserved in the pinned submodule:

- `extern/CommonLibVR/COPYING`
- `extern/CommonLibVR/EXCEPTIONS.md`

They are also included in packaged binary releases. This treatment of the combined
DLL does not withdraw the separate Unlicense grant for WorldMapSelector-authored
material when that material is reused independently.

## Other dependencies

Other third-party components retain their own licenses. See
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for the dependency list and
[SOURCES.md](SOURCES.md) for release-to-source and dependency-reproduction details.
