#pragma once

namespace WMS::MapChooser
{
    // Opens the chooser. Does nothing unless MapMenu is open (and no other message box is up).
    void Open();
    bool Dismiss();
    // True while the chooser's message box is showing.
    bool IsActive();
}
