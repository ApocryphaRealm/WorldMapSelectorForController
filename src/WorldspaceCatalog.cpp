#include "Config.h"
#include "Utilities.h"
#include "WorldspaceCatalog.h"

#include <shared_mutex>

namespace WMS::WorldspaceCatalog
{
    namespace
    {
        // entries is shared by the builder and the UI/command readers. It holds every world found, hidden ones too.
        // shared_mutex permits multiple readers but only one writer.
        std::vector<MapOption> entries;
        std::shared_mutex entriesLock;

        // Convert Skyrim's borrowed C string into an owned std::string.
        std::string SafeText(const char* text, std::string_view fallback)
        {
            return text && text[0] ? text : std::string(fallback);
        }

        // Mark every option that shares its displayed name with another map.
        // This runs once while building the catalog rather than for every chooser button.
        void MarkDuplicateDisplayNames(std::vector<MapOption>& options)
        {
            for (std::size_t left = 0; left < options.size(); ++left) {
                for (std::size_t right = left + 1; right < options.size(); ++right) {
                    if (Utilities::EqualsIgnoreCase(options[left].displayName, options[right].displayName)) {
                        options[left].hasDuplicateDisplayName  = true;
                        options[right].hasDuplicateDisplayName = true;
                    }
                }
            }
        }

        // Sort alphabetically by display name, using EditorID to make equal names deterministic.
        bool CompareMapOptions(const MapOption& left, const MapOption& right)
        {
			// stricmp is a Microsoft extension that compares two C strings case-insensitively.
			// It returns a negative value if left < right, zero if equal, and a positive value if left > right.
            const auto nameComparison = _stricmp(left.displayName.c_str(), right.displayName.c_str());
            if (nameComparison != 0) {
                return nameComparison < 0;
            } else {
				const auto editorIDComparison = _stricmp(left.editorID.c_str(), right.editorID.c_str());
                return editorIDComparison < 0;
            }
        }

        // Move one worldspace to a preferred position while preserving the order of the other options.
        bool MoveToIndex(std::vector<MapOption>& options, RE::TESWorldSpace* worldspace, std::size_t index)
        {
            if (!worldspace || index >= options.size()) return false;

            const auto destination = std::next(options.begin(), index);

            // The projection &MapOption::worldspace tells ranges::find to compare worldspace against that member of each MapOption.
            const auto option = std::ranges::find(destination, options.end(), worldspace, &MapOption::worldspace);
            if (option == options.end()) return false;

			// std::rotate moves the element at option to destination, shifting the intervening elements to the right.
            std::rotate(destination, option, std::next(option));
            return true;
        }

    }

    // Called after SKSE's DataLoaded message, when forms from every active plugin are available and their runtime FormIDs are final.
    void Build()
    {
        auto* dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) {
            SKSE::log::error("Could not get the data handler for the map catalog.");
            return;
        }

        std::vector<MapOption> newEntries;

        // Range-based for visits every loaded TESWorldSpace form.
        for (auto* worldspace : dataHandler->GetFormArray<RE::TESWorldSpace>()) {
            if (!OwnsUsableMap(worldspace)) continue;

            const auto editorID = SafeText(worldspace->GetFormEditorID(), "");
            if (!Config::IsWorldspaceIncluded(editorID)) {
                SKSE::log::debug("Map candidate is not included: editorID=\"{}\", FormID={:08X}.", editorID, worldspace->GetFormID());
                continue;
            }

            const auto* file = worldspace->GetFile(0);
            if (!file) {
                SKSE::log::warn("Ignoring dynamic map candidate {:08X}; it has no originating plugin.", worldspace->GetFormID());
                continue;
            }

            // An override from the INI wins: a record's name is not always what the map should be called
            // (BSHeartland's says Tamriel, and the land is Bruma).
            auto overrideName = Config::GetWorldspaceNameOverride(editorID);

            // Designated initializers name each MapOption field being filled.
            newEntries.push_back({
                .worldspace  = worldspace,
                .displayName = !overrideName.empty() ? std::move(overrideName) : SafeText(worldspace->GetName(), "<unnamed>"),
                .editorID    = editorID,
                .pluginName  = std::string(file->GetFilename())
            });
        }

        bool changed = false;
        {
            // unique_lock excludes readers while replacing the shared catalogue.
            std::unique_lock lock(entriesLock);
            changed = !std::ranges::equal(entries, newEntries, {}, &MapOption::worldspace, &MapOption::worldspace);
            // move transfers the vector's allocation instead of copying every entry.
            entries = std::move(newEntries);
        }

        // This runs every time a chooser opens; the log only hears about a list that differs from the last one.
        if (!changed) return;

        std::shared_lock lock(entriesLock);
        SKSE::log::info("Built map catalog with {} worldspaces that own a map.", entries.size());
        for (const auto& entry : entries) {
            SKSE::log::info(
                "Map: name=\"{}\", editorID=\"{}\", FormID={:08X}, plugin=\"{}\"",
                entry.displayName, entry.editorID.empty() ? "<none>" : entry.editorID, entry.worldspace->GetFormID(), entry.pluginName
            );
        }
    }

    bool OwnsUsableMap(RE::TESWorldSpace* worldspace)
    {
        // Only worldspaces that own their own map data.
        if (!worldspace || GetMapOwner(worldspace) != worldspace) return false;

        // A usable world map needs non-degenerate cell bounds.
        // This also admits map data supplied to normally mapless worlds by mods.
        const auto& map = worldspace->worldMapData;
        return map.nwCellX != map.seCellX || map.nwCellY != map.seCellY;
    }

    RE::TESWorldSpace* GetMapOwner(RE::TESWorldSpace* worldspace)
    {
        // Child worldspaces such as Whiterun can explicitly reuse their parent's map data.
        // The chooser should list the owning map once.
        auto* owner = worldspace;

        // Follow the parent only when this worldspace explicitly inherits the parent's map data.
        // The final pointer owns the displayed map.
        while (owner && owner->parentWorld && owner->parentUseFlags.any(RE::TESWorldSpace::ParentUseFlag::kUseMapData)) {
            owner = owner->parentWorld;
        }

        return owner;
    }

    std::vector<MapOption> GetOrderedOptions(RE::TESWorldSpace* currentWorldspace, RE::TESWorldSpace* selectedWorldspace)
    {
        Build();

        // Copy a stable snapshot while holding a shared/read lock, then release the lock before sorting the private copy.
        std::shared_lock lock(entriesLock);
        auto result = entries;
        lock.unlock();

        std::erase_if(result, [](const MapOption& option) { return Config::IsWorldspaceExcluded(option.editorID); });

        MarkDuplicateDisplayNames(result);
        std::ranges::sort(result, CompareMapOptions);

        auto* currentMap  = GetMapOwner(currentWorldspace);
        auto* selectedMap = GetMapOwner(selectedWorldspace);

        // Keep the useful status entries first; the remaining entries retain
        // the case-insensitive alphabetical ordering established above.

        const bool currentWasMoved = MoveToIndex(result, currentMap, 0);
        if (selectedMap != currentMap) {
            MoveToIndex(result, selectedMap, currentWasMoved ? 1 : 0);
        }

        return result;
    }

    std::vector<MapOption> GetAllOptions()
    {
        std::shared_lock lock(entriesLock);
        auto result = entries;
        lock.unlock();

        MarkDuplicateDisplayNames(result);
        std::ranges::sort(result, CompareMapOptions);
        return result;
    }
}
