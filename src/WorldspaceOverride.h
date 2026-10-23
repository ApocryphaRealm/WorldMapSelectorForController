#pragma once

namespace WMS::WorldspaceOverride
{
    bool CreateHook();
    void BeginSession();
    void ResetSession();
    RE::TESWorldSpace* GetActualMapWorldspace();
    RE::TESWorldSpace* GetSelectedMapWorldspace();
    // The worldspace an open MapMenu is drawing: the redirect while one is active, else vanilla's answer (the player's world).
    RE::TESWorldSpace* GetShownMapWorldspace();
}
