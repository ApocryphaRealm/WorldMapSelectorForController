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
    bool IsWorldspaceExcluded(std::string_view editorID);
}
