#pragma once

namespace WMS::MapChooserInput
{
    bool Register();

    // True when the last keyboard, mouse or controller input came from the controller (picks the hint's button art).
    bool LastInputWasGamepad();

    struct Counters
    {
        std::uint32_t opensFromKeyboard = 0;
        std::uint32_t opensFromGamepad  = 0;
        std::uint32_t ignoredOutsideMap = 0;
    };

    // How often the trigger opened the chooser, and how often it was pressed outside the map.
    Counters GetCounters();

    // True when this button is the chooser's trigger (MapMenuKey on the keyboard, MapMenuButton on the controller).
    bool IsTrigger(const RE::ButtonEvent& button);

    // The menu framework's input callback (game thread, before the input reaches any other plugin or Papyrus) hands
    // the trigger's press here. Opens the map list panel when the world map is open; true when it opened, and the
    // caller then claims the press so no other mod's key or button acts on it. Outside the world map: false.
    bool OpenPanelFromTrigger(const RE::ButtonEvent& button);
}
