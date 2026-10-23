#pragma once

namespace WMS::Config
{
    void Load();
    // Leaves the keyboard trigger unbound when the menu framework reserves its key. Call once all plugins are loaded.
    void ApplyFrameworkReservedKeys();
    std::uint32_t GetMapMenuKey();    // DirectInput scan code; 0 turns the keyboard trigger off
    std::uint32_t GetMapMenuButton(); // XInput button mask; 0 turns the controller trigger off
    bool GetPersistSelection();
    bool GetShowMapMenuKeyHint();
    bool GetShowMapMenuKeyHintOnLocalMap();
    std::size_t GetMapsPerPage();
    bool GetShowCancelButton();
    bool GetShowClearSelectionButton();
    bool IsWorldspaceIncluded(std::string_view editorID);

    // The map list panel (an Apocrypha Menu Framework window docked to one side of the world map).
    enum class PanelSide : int { kRight = 0, kLeft = 1 };
    inline constexpr int kPanelWidthMin = 15, kPanelWidthMax = 40, kPanelWidthDefault = 22;   // percent of screen width
    inline constexpr int kTextScaleMin = 80, kTextScaleMax = 160, kTextScaleDefault = 100;    // percent
    PanelSide GetPanelSide();
    int GetPanelWidthPercent();
    int GetPanelTextScalePercent();

    // Setters for the settings page. They apply at once; Save() writes them to the INI.
    void SetPanelSide(PanelSide side);
    void SetPanelWidthPercent(int percent);
    void SetPanelTextScalePercent(int percent);
    void SetPersistSelection(bool value);
    void SetShowMapMenuKeyHint(bool value);
    void SetShowMapMenuKeyHintOnLocalMap(bool value);
    void SetShowClearSelectionButton(bool value);
    // Writes the page's settings into the INI (SimpleIni, which works under MO2's virtual file system). False on failure.
    bool Save();
    // Puts the page's settings back to the shipped defaults (in memory; Save keeps them).
    void RestoreDefaults();
    std::string GetIniPath();
    bool IsWorldspaceExcluded(std::string_view editorID);
}
