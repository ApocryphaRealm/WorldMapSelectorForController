#include "Hooks.h"
#include "MapMarkerOverride.h"
#include "WorldspaceOverride.h"

namespace WMS::MapMarkerOverride
{
    namespace
    {
        // Vanilla's marker helpers call one another synchronously.
        // Keeping this state thread-local, prevents one marker rebuild from affecting another thread.
        thread_local RE::BSTArray<RE::ObjectRefHandle> selectedMapMarkerHandles;
        thread_local bool suppressActualWorldMarkers = false;

        // Skyrim's native function to collect map marker handles from a worldspace.
		// Takes the worldspace to collect from, a pointer to the destination array, and a boolean to include hidden markers.
        using CollectMapMarkerHandles_t = void (*)(RE::TESWorldSpace*, RE::BSTArray<RE::ObjectRefHandle>*, bool);
        // Resolved on first use (inside a function), never at static init, so the Address Library is opened only after
        // SKSEPluginLoad's checks have passed (gate row no-address-resolved-at-static-init).
        void CollectMapMarkerHandles(RE::TESWorldSpace* worldspace, RE::BSTArray<RE::ObjectRefHandle>* handles, bool includeHidden)
        {
            static REL::Relocation<CollectMapMarkerHandles_t> function{ REL::RelocationID(20089, 20536) };
            function(worldspace, handles, includeHidden);
        }

		// Skyrim's native function to add the player's current location marker to the map.
		// It takes a pointer to the destination array and a pointer to the player's marker position.
        using AddCurrentLocationMarker_t = void (*)(RE::BSTArray<RE::MapMenuMarker>*, RE::NiPoint3*);
        AddCurrentLocationMarker_t originalAddCurrentLocationMarker = nullptr;

        // Skyrim's native functions to add ordinary markers from handles to the map.
        // AE's helper processes one handle; its caller owns the loop.
        using AddMarkerFromHandleAE_t = std::uint64_t (*)(RE::BSTArray<RE::MapMenuMarker>**, RE::ObjectRefHandle*);
        AddMarkerFromHandleAE_t originalAddMarkerFromHandleAE = nullptr;

        // SE's helper receives the complete handle array and owns the loop itself.
        using AddMarkersFromHandlesSE_t = std::uint64_t (*)(RE::BSTArray<RE::ObjectRefHandle>*, RE::BSTArray<RE::MapMenuMarker>**);
        AddMarkersFromHandlesSE_t originalAddMarkersFromHandlesSE = nullptr;

        std::size_t AppendSelectedMapMarkers(RE::BSTArray<RE::MapMenuMarker>* mapMarkers)
        {
            const auto markerCountBefore = mapMarkers->size();
            auto* destination = mapMarkers;

            if (REL::Module::IsSE()) {
                originalAddMarkersFromHandlesSE(std::addressof(selectedMapMarkerHandles), std::addressof(destination));
            } else {
                for (auto& handle : selectedMapMarkerHandles) {
                    originalAddMarkerFromHandleAE(std::addressof(destination), std::addressof(handle));
                }
            }

            return mapMarkers->size() - markerCountBefore;
        }

        // Detour for vanilla's AddCurrentLocationMarker.
        // Vanilla calls this immediately before processing the player's currentMapMarkers array.
        // For a remote map, append markers collected from the selected worldspace, and omit the player's location marker.
        void AddCurrentLocationMarkerHook(RE::BSTArray<RE::MapMenuMarker>* mapMarkers, RE::NiPoint3* playerMarkerPosition)
        {
            suppressActualWorldMarkers = false;

            auto* selectedWorldspace = WorldspaceOverride::GetSelectedMapWorldspace();

			// If we're not processing a remote map, call the original function to add the player's current location marker.
            if (!selectedWorldspace) {
                originalAddCurrentLocationMarker(mapMarkers, playerMarkerPosition);
                return;
            }

            if (!mapMarkers) return;

			// Collect all map marker handles from the selected worldspace.
			selectedMapMarkerHandles.clear();
            CollectMapMarkerHandles(selectedWorldspace, std::addressof(selectedMapMarkerHandles), false);

            // Append the selected handles through this runtime's original ordinary-marker pipeline.
            const auto markersAdded = AppendSelectedMapMarkers(mapMarkers);

            suppressActualWorldMarkers = true;

            SKSE::log::debug(
                "Remote map {:08X}: omitted current location; collected {} handles and added {} markers.",
                selectedWorldspace->GetFormID(), selectedMapMarkerHandles.size(), markersAdded
            );
        }

		// AE detour for vanilla's AddMarkerFromHandle.
        // Returning zero tells AE's caller-owned currentMapMarkers loop to stop.
        // The selected markers were already appended by AddCurrentLocationMarkerHook.
        std::uint64_t AddMarkerFromHandleAEHook(RE::BSTArray<RE::MapMenuMarker>** destination, RE::ObjectRefHandle* handle)
        {
            if (suppressActualWorldMarkers) {
                suppressActualWorldMarkers = false;
                return 0;
            } else {
                return originalAddMarkerFromHandleAE(destination, handle);
            }
        }

        // SE calls this helper once with the entire actual-world handle array.
        // The selected array was already processed above, so suppress that one complete vanilla pass.
        std::uint64_t AddMarkersFromHandlesSEHook(RE::BSTArray<RE::ObjectRefHandle>* handles, RE::BSTArray<RE::MapMenuMarker>** destination)
        {
            if (suppressActualWorldMarkers) {
                suppressActualWorldMarkers = false;
                return 1;
            } else {
                return originalAddMarkersFromHandlesSE(handles, destination);
            }
        }
    }

    // Creates the ordinary world-marker detours in a disabled state.
    bool CreateHooks()
    {
        if (!Hooks::Create(
            "AddCurrentLocationMarker",
            REL::RelocationID(52184, 53076),
            AddCurrentLocationMarkerHook,
            originalAddCurrentLocationMarker
        )) return false;

        if (REL::Module::IsSE()) {
            return Hooks::Create(
                "AddMarkersFromHandles",
                REL::ID(52235),
                AddMarkersFromHandlesSEHook,
                originalAddMarkersFromHandlesSE
            );
        } else {
            return Hooks::Create(
                "AddMarkerFromHandle",
                REL::ID(53126),
                AddMarkerFromHandleAEHook,
                originalAddMarkerFromHandleAE
            );
        }
    }
}
