#include "Hooks.h"
#include "MapSelection.h"
#include "WorldspaceOverride.h"

namespace WMS::WorldspaceOverride
{
    namespace
    {
        // Non-null only when the current MapMenu is being redirected to a worldspace other than the one vanilla resolved.
        // Marker hooks use this value to distinguish a remote map from normal map operation.
        std::atomic<RE::TESWorldSpace*> selectedMapWorldspace = nullptr;

        // Resolver calls made during one MapMenu lifetime must agree.
        // The mutex prevents two threads from changing or evaluating the session together.
        std::mutex sessionLock;
        bool sessionActive = false;

        std::mutex selectionLogLock;
        std::string lastSelectionLog;

        // This alias describes a pointer to a function taking no arguments and returning TESWorldSpace*.
        // MinHook stores vanilla's callable function here.
        using ResolveMapWorldSpace_t = RE::TESWorldSpace* (*)();
        ResolveMapWorldSpace_t originalResolveMapWorldSpace = nullptr;

        bool IsNewSelectionLog(std::string_view message)
        {
            std::scoped_lock lock(selectionLogLock);

            if (lastSelectionLog == message) {
                return false;
            } else {
                lastSelectionLog = message;
                return true;
            }
        }

        RE::TESWorldSpace* ResolveMapWorldSpaceHook()
        {
            // Call the trampoline supplied by MinHook to obtain vanilla's answer.
            auto* actualWorldspace = originalResolveMapWorldSpace();

            // This resolver is also called by world-map resource lifecycle
            // code outside an open menu. Only an active session is frozen;
            // otherwise reevaluate the player's current requested selection.

            std::scoped_lock sessionGuard(sessionLock);

            if (sessionActive) {
                if (auto* selectedWorldspace = selectedMapWorldspace.load(std::memory_order_acquire)) {
                    return selectedWorldspace;
                }

                return actualWorldspace;
            }

            auto* requestedWorldspace = MapSelection::GetSelectedWorldspace();
            if (!requestedWorldspace || requestedWorldspace == actualWorldspace) {
                selectedMapWorldspace.store(nullptr, std::memory_order_release);

                // The conditional operator constructs the appropriate log text.
                const auto description = !requestedWorldspace ? std::string("Default") : fmt::format("Default (requested map {:08X} is current)", requestedWorldspace->GetFormID());

                if (IsNewSelectionLog(description)) {
                    SKSE::log::info("Map selection: {}.", description);
                }
                return actualWorldspace;
            }

            selectedMapWorldspace.store(requestedWorldspace, std::memory_order_release);

            const auto* editorID = requestedWorldspace->GetFormEditorID();
            const auto description = fmt::format("{} -> {:08X}", editorID && editorID[0] ? editorID : "<no EditorID>", requestedWorldspace->GetFormID());
            if (IsNewSelectionLog(description)) {
                SKSE::log::info("Map selection: {}.", description);
            }

            return requestedWorldspace;
        }
    }

    RE::TESWorldSpace* GetActualMapWorldspace()
    {
        // Never call through a null function pointer if installation failed or this function is reached before the detour has been created.
        return originalResolveMapWorldSpace ? originalResolveMapWorldSpace() : nullptr;
    }

    RE::TESWorldSpace* GetSelectedMapWorldspace()
    {
        return selectedMapWorldspace.load(std::memory_order_acquire);
    }

    RE::TESWorldSpace* GetShownMapWorldspace()
    {
        if (auto* redirected = GetSelectedMapWorldspace()) {
            return redirected;
        }
        return GetActualMapWorldspace();
    }

    void BeginSession()
    {
        // The constructor's resolver call established selectedMapWorldspace
        // immediately before the MapMenu open event reaches this function.
        std::scoped_lock sessionGuard(sessionLock);

        sessionActive = true;
        const auto* selectedWorldspace = selectedMapWorldspace.load(std::memory_order_acquire);

        SKSE::log::debug("Froze map selection for open session: {:08X}.", selectedWorldspace ? selectedWorldspace->GetFormID() : 0);
    }

    void ResetSession()
    {
        std::scoped_lock sessionGuard(sessionLock);

        const auto wasActive = sessionActive;
        sessionActive = false;
        selectedMapWorldspace.store(nullptr, std::memory_order_release);

        if (wasActive) {
            SKSE::log::debug("Cleared map selection session state.");
        }
    }

    bool CreateHook()
    {
        return Hooks::Create("WMS_ResolveMapWorldSpace", REL::RelocationID(52260, 53150), ResolveMapWorldSpaceHook, originalResolveMapWorldSpace);
    }
}
