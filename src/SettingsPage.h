#pragma once

// The mod's settings page in the Apocrypha Menu Framework (SKSE Menu Framework-compatible): which side of the
// world map the list panel docks to, its width and text size, the behaviour switches and one switch per world found. Changes apply at once
// and are written to WorldMapSelectorForController.ini.
namespace WMS::SettingsPage
{
    // Call once all plugins have loaded (kPostPostLoad). Without a framework the INI is the only settings surface.
    void Register();

    // DevBench driving (rule 64): the same setters the page's controls call, then a save.
    void ApplyFromTool(const std::string& side, int widthPercent, int textScalePercent);
    // The same as a world's switch on the page, then a save. False when no world found has that EditorID.
    bool SetWorldHiddenFromTool(std::string_view editorID, bool hidden);
}
