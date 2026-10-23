#pragma once

// The map list panel: a vertical, scrollable list of every selectable world map, docked to one side of the
// world map (right by default) and as tall as the map area. Drawn as an Apocrypha Menu Framework window
// (the SKSE Menu Framework-compatible AddWindow surface), which hands the panel the controller, keyboard and
// mouse while it is open, so the map behind it does not move. Without the framework the message-box chooser
// (MapChooser) is used instead.
namespace WMS::MapPanel
{
    // Registers the window with the menu framework. Call once all plugins have loaded (kPostPostLoad).
    // False when no framework (or one too old for the panel) is installed.
    bool Register();

    // True when the panel was registered and is used instead of the message-box chooser.
    bool IsAvailable();

    // Opens the panel. Does nothing unless MapMenu is open and no message box is up. Main thread.
    void Open();

    // Closes the panel without choosing anything. True when it was open.
    bool Dismiss();

    // True from Open until the panel has handed the input back.
    bool IsOpen();

    // MapMenu closed underneath the panel: close it at once.
    void OnMapMenuClosed();

    // DevBench driving (rule 64): queue a highlight move (+down / -up), a choice of the highlighted row,
    // and test-only fake rows that prove the list scrolls (0 removes them; never shipped enabled).
    void QueueNavigate(int rows);
    void QueueSelect();
    void SetFakeEntryCount(int count);

    // The panel's live state as a JSON object, for the DevBench tool (rule 31).
    std::string StateJson();
}
