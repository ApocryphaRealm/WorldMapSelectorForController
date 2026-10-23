#include "Config.h"
#include "Utilities.h"

#include <charconv>
#include <SimpleIni.h>

namespace WMS::Config
{
    namespace
    {
        // Turn the game-relative INI path into the path SimpleIni reads.
        const auto configPath = std::filesystem::absolute("Data\\SKSE\\Plugins\\WorldMapSelectorForController.ini").string();

        // Initialize the plugin configuration settings with their default values.
        // Missing or invalid INI entries leave these values unchanged.
        auto logLevel                 = spdlog::level::info; // This default actually needs to be applied
        // The chooser opens only while MapMenu is open. These two defaults must match the shipped INI (rule 16)
        // and DEFAULT-KEYS.md.
        std::uint32_t mapMenuKey      = 0x2E;   // DirectInput scan code: C
        std::uint32_t mapMenuButton   = 0x0001; // XInput button mask: D-pad up
        bool persistSelection         = true;
        bool showMapMenuKeyHint           = true;
        bool showMapMenuKeyHintOnLocalMap = false;
        std::size_t mapsPerPage           = 6;
        bool showCancelButton             = true;
        bool showClearSelectionButton     = true;
        std::vector<std::string> includedWorldspaces;
        std::vector<std::string> excludedWorldspaces { "Falskaar" };

        std::optional<spdlog::level::level_enum> ParseLogLevel(std::string_view text)
        {
            if (Utilities::EqualsIgnoreCase(text, "trace"))    return spdlog::level::trace;
            if (Utilities::EqualsIgnoreCase(text, "debug"))    return spdlog::level::debug;
            if (Utilities::EqualsIgnoreCase(text, "info"))     return spdlog::level::info;
            if (Utilities::EqualsIgnoreCase(text, "warn"))     return spdlog::level::warn;
            if (Utilities::EqualsIgnoreCase(text, "err"))      return spdlog::level::err;
            if (Utilities::EqualsIgnoreCase(text, "critical")) return spdlog::level::critical;
            if (Utilities::EqualsIgnoreCase(text, "off"))      return spdlog::level::off;
            return std::nullopt;
        }

        std::optional<std::uint32_t> ParseKeyCode(std::string_view text, std::uint32_t maximum)
        {
            // Require the hexadecimal prefix documented in the INI,
            // then remove it from this non-owning view before parsing the remaining digits.
            if (!Utilities::HasHexPrefix(text)) return std::nullopt;
            text.remove_prefix(2);
            if (text.empty()) return std::nullopt;

            std::uint32_t parsed = 0;
            const char* start    = text.data();
            const char* end      = start + text.size();
            const auto result    = std::from_chars(start, end, parsed, 16);

            // from_chars reports both a parsing error and where parsing stopped.
            // Require every character to be valid hex and the result to fit the device's code range.
            if (result.ec != std::errc{} || result.ptr != end || parsed > maximum) {
                return std::nullopt;
            }

            return parsed;
        }

        std::optional<bool> ParseBool(std::string_view text)
        {
            if (Utilities::EqualsIgnoreCase(text, "true"))  return true;
            if (Utilities::EqualsIgnoreCase(text, "false")) return false;
            return std::nullopt;
        }

        std::optional<std::size_t> ParseMapsPerPage(std::string_view text)
        {
            std::size_t parsed = 0;
            const char* start  = text.data();
            const char* end    = start + text.size();
            const auto result  = std::from_chars(start, end, parsed, 10);

            if (result.ec != std::errc{} || result.ptr != end || parsed < 1 || parsed > 7) {
                return std::nullopt;
            }

            return parsed;
        }

        std::string_view Trim(std::string_view text)
        {
            const auto isWhitespace = [](char character) {
                return character == ' ' || character == '\t' || character == '\r' || character == '\n';
            };

            while (!text.empty() && isWhitespace(text.front())) text.remove_prefix(1);
            while (!text.empty() && isWhitespace(text.back())) text.remove_suffix(1);
            return text;
        }

        // Set the global log level and flush policy for spdlog.
        void ApplyLogLevel()
        {
            spdlog::set_level(logLevel);
            spdlog::default_logger()->flush_on(logLevel);
        }

        void ReadLogLevel(const CSimpleIniA& ini)
        {
            if (const char* configured = ini.GetValue("General", "LogLevel")) {
                if (const auto parsed = ParseLogLevel(configured)) {
                    logLevel = *parsed;
                }
                else { SKSE::log::warn("Invalid LogLevel \"{}\"; keeping default Info.", configured); }
            }
            else { SKSE::log::debug("LogLevel not specified; keeping default Info."); }

            ApplyLogLevel();
        }

        // Reads one [Controls] code. 0 turns that device's trigger off. A missing or invalid value keeps the default.
        void ReadControlCode(const CSimpleIniA& ini, const char* key, std::uint32_t maximum, std::uint32_t& setting)
        {
            if (const char* configured = ini.GetValue("Controls", key)) {
                if (const auto parsed = ParseKeyCode(configured, maximum)) {
                    setting = *parsed;
                    SKSE::log::debug("{} read as 0x{:X}.", key, setting);
                }
                else { SKSE::log::warn("Invalid {} \"{}\"; keeping default 0x{:X}.", key, configured, setting); }
            }
            else { SKSE::log::debug("{} not specified; keeping default 0x{:X}.", key, setting); }
        }

        // Keys the original World Map Selector read. They do nothing here; say so once so a copied INI is no mystery.
        void WarnRetiredKeys(const CSimpleIniA& ini)
        {
            if (ini.GetValue("Controls", "OpenSelectorKey")) {
                SKSE::log::warn("OpenSelectorKey is not used; the chooser opens from the world map with MapMenuKey / MapMenuButton.");
            }
            for (const char* key : { "OpenMapAfterSelection", "AllowChooserOutsideMap", "AllowChooserWhileMapOpen" }) {
                if (ini.GetValue("Behavior", key)) {
                    SKSE::log::warn("{} is not used; the chooser opens only while the world map is open.", key);
                }
            }
        }

        void ReadBool(const CSimpleIniA& ini, const char* key, bool& setting)
        {
            if (const char* configured = ini.GetValue("Behavior", key)) {
                if (const auto parsed = ParseBool(configured)) {
                    setting = *parsed;
                }
                else { SKSE::log::warn("Invalid {} \"{}\"; expected true or false. Keeping default.", key, configured); }
            }
            else { SKSE::log::debug("{} not specified. Keeping default.", key); }
        }

        void ReadMapsPerPage(const CSimpleIniA& ini)
        {
            if (const char* configured = ini.GetValue("Behavior", "MapsPerPage")) {
                if (const auto parsed = ParseMapsPerPage(configured)) {
                    mapsPerPage = *parsed;
                }
                else { SKSE::log::warn("Invalid MapsPerPage \"{}\"; expected a number from 1 through 7. Keeping default 6.", configured); }
            }
            else { SKSE::log::debug("MapsPerPage not specified. Keeping default 6."); }
        }

        void ReadWorldspaceList(const CSimpleIniA& ini, const char* key, std::vector<std::string>& worldspaces)
        {
            const char* configured = ini.GetValue("Compatibility", key);
            if (!configured) {
                SKSE::log::debug("{} not specified. Keeping default.", key);
                return;
            }

            worldspaces.clear();
            std::string_view remaining = configured;

            while (true) {
                const auto separator = remaining.find(',');
                const auto editorID = Trim(remaining.substr(0, separator));
                if (!editorID.empty()) worldspaces.emplace_back(editorID);

                if (separator == std::string_view::npos) break;
                remaining.remove_prefix(separator + 1);
            }
        }
    }

    // Load the INI file and replace defaults only with settings that exist and parse successfully.
    void Load()
    {
		// Read the INI file into memory.
		// SimpleIni's LoadFile returns a status code, which is negative on error and non-negative on success.
        CSimpleIniA ini;
        const auto loadResult = ini.LoadFile(configPath.c_str());
        if (loadResult < SI_OK) {
            SKSE::log::warn("Could not load {}; keeping all default settings (error {}).", configPath, loadResult);
			ApplyLogLevel();
            return;
        }

		// Read each setting from the INI file, leaving the default value unchanged if the key is missing or invalid.
        ReadLogLevel(ini);
        ReadControlCode(ini, "MapMenuKey", 0xFF, mapMenuKey);
        ReadControlCode(ini, "MapMenuButton", 0xFFFF, mapMenuButton);
        WarnRetiredKeys(ini);
        ReadBool(ini, "ShowMapMenuKeyHint", showMapMenuKeyHint);
        ReadBool(ini, "ShowMapMenuKeyHintOnLocalMap", showMapMenuKeyHintOnLocalMap);
        ReadMapsPerPage(ini);
        ReadBool(ini, "ShowCancelButton", showCancelButton);
        ReadBool(ini, "ShowClearSelectionButton", showClearSelectionButton);
        ReadBool(ini, "PersistSelection", persistSelection);
        ReadWorldspaceList(ini, "IncludedWorldspaces", includedWorldspaces);
        ReadWorldspaceList(ini, "ExcludedWorldspaces", excludedWorldspaces);

        SKSE::log::info(
            "Loaded configuration: LogLevel={}, MapMenuKey=0x{:02X}, MapMenuButton=0x{:04X}, ShowMapMenuKeyHint={}, ShowMapMenuKeyHintOnLocalMap={}, MapsPerPage={}, ShowCancelButton={}, ShowClearSelectionButton={}, PersistSelection={}, IncludedWorldspaces={}, ExcludedWorldspaces={}.",
            spdlog::level::to_string_view(logLevel), mapMenuKey, mapMenuButton, showMapMenuKeyHint, showMapMenuKeyHintOnLocalMap, mapsPerPage, showCancelButton, showClearSelectionButton, persistSelection,
            includedWorldspaces.empty() ? "<all>" : fmt::format("{} entr{}", includedWorldspaces.size(), includedWorldspaces.size() == 1 ? "y" : "ies"),
            excludedWorldspaces.empty() ? "<none>" : fmt::format("{} entr{}", excludedWorldspaces.size(), excludedWorldspaces.size() == 1 ? "y" : "ies")
        );
    }

    // Rule 28: a keyboard default or INI value on a key the Apocrypha Menu Framework reserves is left unbound,
    // with a warning. Asked of the framework itself (SMF_GetReservedKeyCodes), so it follows the framework's live key.
    void ApplyFrameworkReservedKeys()
    {
        REX::W32::HMODULE framework = nullptr;
        for (const wchar_t* name : { L"!ApocryphaMenuFramework", L"ApocryphaMenuFramework", L"SKSEMenuFramework" }) {
            if ((framework = REX::W32::GetModuleHandleW(name))) break;
        }
        if (!framework) {
            SKSE::log::debug("No menu framework loaded; no reserved keys to avoid.");
            return;
        }

        using func_t = std::uint32_t (*)(std::int32_t*, std::uint32_t);
        const auto func = reinterpret_cast<func_t>(REX::W32::GetProcAddress(framework, "SMF_GetReservedKeyCodes"));
        if (!func) {
            SKSE::log::debug("The menu framework does not export SMF_GetReservedKeyCodes; no reserved keys to avoid.");
            return;
        }

        std::vector<std::int32_t> reserved(func(nullptr, 0));
        if (!reserved.empty()) reserved.resize(func(reserved.data(), static_cast<std::uint32_t>(reserved.size())));
        SKSE::log::debug("The menu framework reserves {} key(s).", reserved.size());

        if (mapMenuKey != 0 && std::ranges::find(reserved, static_cast<std::int32_t>(mapMenuKey)) != reserved.end()) {
            SKSE::log::warn("MapMenuKey 0x{:02X} is reserved by the menu framework; the keyboard trigger is left unbound.", mapMenuKey);
            mapMenuKey = 0;
        }
    }

    // Public getters return copies so callers cannot modify the stored settings.
    std::uint32_t GetMapMenuKey()      { return mapMenuKey; }
    std::uint32_t GetMapMenuButton()   { return mapMenuButton; }
    bool GetPersistSelection()         { return persistSelection; }
    bool GetShowMapMenuKeyHint()           { return showMapMenuKeyHint; }
    bool GetShowMapMenuKeyHintOnLocalMap() { return showMapMenuKeyHintOnLocalMap; }
    std::size_t GetMapsPerPage()           { return mapsPerPage; }
    bool GetShowCancelButton()             { return showCancelButton; }
    bool GetShowClearSelectionButton()     { return showClearSelectionButton; }
    bool IsWorldspaceIncluded(std::string_view editorID)
    {
        return includedWorldspaces.empty() || std::ranges::any_of(includedWorldspaces, [&](const auto& included) { return Utilities::EqualsIgnoreCase(editorID, included); });
    }
    bool IsWorldspaceExcluded(std::string_view editorID)
    {
        return std::ranges::any_of(excludedWorldspaces, [&](const auto& excluded) { return Utilities::EqualsIgnoreCase(editorID, excluded); });
    }
}
