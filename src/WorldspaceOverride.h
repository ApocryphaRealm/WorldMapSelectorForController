#pragma once

namespace WMS::WorldspaceOverride
{
    bool CreateHook();
    void BeginSession();
    void ResetSession();
    // Vanilla's answer. In a child world that owns its map that is still the parent's map.
    RE::TESWorldSpace* GetActualMapWorldspace();
    // The map that is the player's own: their world when it owns a map vanilla would not give them, else vanilla's answer.
    RE::TESWorldSpace* GetPlayerMapWorldspace();
    RE::TESWorldSpace* GetSelectedMapWorldspace();
    // The worldspace an open MapMenu is drawing: the redirect while one is active, else vanilla's answer (the player's world).
    RE::TESWorldSpace* GetShownMapWorldspace();
}
