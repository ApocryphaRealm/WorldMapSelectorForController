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
}
