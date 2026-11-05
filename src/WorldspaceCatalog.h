#pragma once

namespace WMS::WorldspaceCatalog
{
    // Each option owns its display strings but only borrows the TESWorldSpace;
    // Skyrim owns that form for the lifetime of the loaded game data.
    struct MapOption
    {
        RE::TESWorldSpace* worldspace = nullptr;
        std::string displayName;
        std::string editorID;
        std::string pluginName;
        bool hasDuplicateDisplayName = false;
    };

    // Finds every world that owns a map right now. Another plugin may give a world its own map at any point up to
    // the map opening, so this is run again each time a chooser asks for its list.
    void Build();
    RE::TESWorldSpace* GetMapOwner(RE::TESWorldSpace* worldspace);
    // This world does not take its parent's map, and has map bounds to draw: the list's test, and the redirect's.
    bool OwnsUsableMap(RE::TESWorldSpace* worldspace);
    // What a chooser lists: the worlds found by a fresh Build, without the hidden ones. Main thread.
    std::vector<MapOption> GetOrderedOptions(RE::TESWorldSpace* currentWorldspace, RE::TESWorldSpace* selectedWorldspace);
    // Every world found, hidden ones too, by name: the settings page's switches. Any thread.
    std::vector<MapOption> GetAllOptions();
}
