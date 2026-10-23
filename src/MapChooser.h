#pragma once

namespace WMS::MapChooser
{
    // Opens the chooser. Does nothing unless MapMenu is open (and no other message box is up).
    void Open();
    bool Dismiss();
    // True while the chooser's message box is showing.
    bool IsActive();
    // The player's own worldspace (vanilla's map answer, before any redirect) - the [Here] map. It is NOT always the map
    // on screen: while another map is shown, WorldspaceOverride::GetShownMapWorldspace() is that one.
    RE::TESWorldSpace* CurrentWorldspace();
}
