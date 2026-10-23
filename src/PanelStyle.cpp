// The map list panel's optional style file (see PanelStyle.h).
//
// Threads: Load runs once on the main thread before the panel is registered; everything else runs on the render
// thread inside the framework's ImGui frame and only reads what Load wrote.

#define NOMINMAX
#include "SKSEMenuFramework.h"

#include "PanelStyle.h"

#include <SimpleIni.h>

namespace WMS::PanelStyle
{
    namespace
    {
        const auto stylePath = std::filesystem::absolute("Data\\SKSE\\Plugins\\WorldMapSelectorForController_PanelStyle.ini").string();

        struct Colour
        {
            bool set = false;
            float r = 0, g = 0, b = 0, a = 1;
            ImGuiMCP::ImVec4 Vec4(float alphaMul = 1.0f) const { return ImGuiMCP::ImVec4{ r, g, b, a * alphaMul }; }
        };

        struct Style
        {
            bool active = false;
            std::string name;
            Colour background;      // the panel's fill
            Colour text;            // row and hint text at rest
            Colour textHighlighted; // the highlighted row's text and the title
            Colour textDim;         // the hint line
            Colour line;            // the panel's edge lines and the separators
            Colour highlightBand;   // the highlighted row's band (fades in from the left, out to the right)
            Colour highlightLine;   // the highlighted row's lines above and below (strongest at the left, gone at the right)
            Colour highlightMark;   // a thin upright mark at the highlighted row's left end
            Colour scrollbar;       // the scroll bar's grab
            bool themeFrame = true; // draw the framework theme's frame (false: the style's own edge lines)
            float lineThickness = 1.0f;
        } style;

        bool exportsChecked = false;
        bool exportsOk = false;
        int pushedColours = 0;
        int pushedVars = 0;

        // #RRGGBB or #RRGGBBAA (the leading # optional).
        bool ParseColour(std::string_view text, Colour& out)
        {
            while (!text.empty() && (text.front() == ' ' || text.front() == '\t')) text.remove_prefix(1);
            while (!text.empty() && (text.back() == ' ' || text.back() == '\t' || text.back() == '\r')) text.remove_suffix(1);
            if (!text.empty() && text.front() == '#') text.remove_prefix(1);
            if (text.size() != 6 && text.size() != 8) return false;
            std::uint32_t value = 0;
            const auto result = std::from_chars(text.data(), text.data() + text.size(), value, 16);
            if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) return false;
            if (text.size() == 6) value = (value << 8) | 0xFF;
            out.r = static_cast<float>((value >> 24) & 0xFF) / 255.0f;
            out.g = static_cast<float>((value >> 16) & 0xFF) / 255.0f;
            out.b = static_cast<float>((value >> 8) & 0xFF) / 255.0f;
            out.a = static_cast<float>(value & 0xFF) / 255.0f;
            out.set = true;
            return true;
        }

        void ReadColour(const CSimpleIniA& ini, const char* key, Colour& out)
        {
            const char* value = ini.GetValue("PanelStyle", key);
            if (!value) {
                SKSE::log::debug("Panel style: {} not set; the framework theme's colour is kept.", key);
                return;
            }
            if (!ParseColour(value, out)) {
                SKSE::log::warn("Panel style: {} = \"{}\" is not #RRGGBB or #RRGGBBAA; ignored.", key, value);
                return;
            }
            SKSE::log::debug("Panel style: {} = #{:02X}{:02X}{:02X}{:02X}.", key, static_cast<int>(out.r * 255.0f + 0.5f),
                static_cast<int>(out.g * 255.0f + 0.5f), static_cast<int>(out.b * 255.0f + 0.5f), static_cast<int>(out.a * 255.0f + 0.5f));
        }

        ImGuiMCP::ImU32 U32(const Colour& c, float alphaMul = 1.0f)
        {
            return ImGuiMCP::ColorConvertFloat4ToU32(c.Vec4(alphaMul));
        }

        void PushColour(ImGuiMCP::ImGuiCol idx, const Colour& c)
        {
            if (!c.set) return;
            ImGuiMCP::PushStyleColor(idx, c.Vec4());
            ++pushedColours;
        }

        // A horizontal line from x0 to x1 that is solid in the middle and fades to nothing over `fade` at each end -
        // how Norden draws its lines (item card edge, the list's selection lines).
        void FadedLine(ImGuiMCP::ImDrawList* list, float x0, float x1, float y, float thickness, const Colour& c, float alphaMul)
        {
            using namespace ImGuiMCP::ImDrawListManager;
            const float fade = (std::min)((x1 - x0) * 0.15f, 120.0f);
            const auto solid = U32(c, alphaMul);
            const auto clear = U32(c, 0.0f);
            AddRectFilledMultiColor(list, ImGuiMCP::ImVec2{ x0, y }, ImGuiMCP::ImVec2{ x0 + fade, y + thickness }, clear, solid, solid, clear);
            AddRectFilled(list, ImGuiMCP::ImVec2{ x0 + fade, y }, ImGuiMCP::ImVec2{ x1 - fade, y + thickness }, solid, 0.0f, 0);
            AddRectFilledMultiColor(list, ImGuiMCP::ImVec2{ x1 - fade, y }, ImGuiMCP::ImVec2{ x1, y + thickness }, solid, clear, clear, solid);
        }
    }

    void Load()
    {
        style = Style{};
        std::error_code ec;
        if (!std::filesystem::exists(stylePath, ec) || ec) {
            SKSE::log::info("No panel style file ({}); the map list panel wears the menu framework's theme.", stylePath);
            return;
        }

        CSimpleIniA ini;
        ini.SetUnicode();
        if (ini.LoadFile(stylePath.c_str()) < 0) {
            SKSE::log::warn("Could not read the panel style file {}; the map list panel wears the menu framework's theme.", stylePath);
            return;
        }

        const char* name = ini.GetValue("PanelStyle", "sName", "unnamed");
        style.name = name ? name : "unnamed";
        ReadColour(ini, "sBackground", style.background);
        ReadColour(ini, "sText", style.text);
        ReadColour(ini, "sTextHighlighted", style.textHighlighted);
        ReadColour(ini, "sTextDim", style.textDim);
        ReadColour(ini, "sLine", style.line);
        ReadColour(ini, "sHighlightBand", style.highlightBand);
        ReadColour(ini, "sHighlightLine", style.highlightLine);
        ReadColour(ini, "sHighlightMark", style.highlightMark);
        ReadColour(ini, "sScrollbar", style.scrollbar);
        style.themeFrame = ini.GetBoolValue("PanelStyle", "bThemeFrame", true);
        const double thickness = ini.GetDoubleValue("PanelStyle", "fLineThickness", 1.0);
        style.lineThickness = static_cast<float>(std::clamp(thickness, 0.5, 6.0));
        style.active = true;
        SKSE::log::info("Panel style \"{}\" read from {} (theme frame {}, line thickness {:.1f}).", style.name, stylePath,
            style.themeFrame ? "on" : "off", style.lineThickness);
    }

    bool IsActive() { return style.active; }

    const std::string& Name() { return style.name; }

    bool HasRequiredExports()
    {
        if (exportsChecked) return exportsOk;
        exportsChecked = true;
        constexpr const char* required[] = {
            "igPushStyleColor_Vec4", "igPopStyleColor", "igPushStyleVar_Float", "igPopStyleVar", "igGetWindowDrawList",
            "igGetCursorScreenPos", "igGetContentRegionAvail", "igGetTextLineHeight", "igColorConvertFloat4ToU32", "ImDrawList_AddRectFilled",
            "ImDrawList_AddRectFilledMultiColor", "igDummy"
        };
        for (const char* exportName : required) {
            if (!GetMenuFrameworkFunction<void*>(exportName)) {
                SKSE::log::warn("Panel style \"{}\": the menu framework does not export \"{}\"; the panel wears the framework's theme.", style.name, exportName);
                exportsOk = false;
                return false;
            }
        }
        exportsOk = true;
        return true;
    }

    void Push()
    {
        pushedColours = 0;
        pushedVars = 0;
        if (!style.active || !HasRequiredExports()) return;
        using namespace ImGuiMCP;
        PushColour(ImGuiCol_WindowBg, style.background);
        PushColour(ImGuiCol_Text, style.text);
        PushColour(ImGuiCol_TextDisabled, style.textDim);
        PushColour(ImGuiCol_Separator, style.line);
        PushColour(ImGuiCol_ScrollbarGrab, style.scrollbar);
        PushColour(ImGuiCol_ScrollbarGrabHovered, style.scrollbar);
        PushColour(ImGuiCol_ScrollbarGrabActive, style.scrollbar);
        if (style.highlightBand.set || style.highlightLine.set || style.highlightMark.set) {
            // The style draws its own highlight, so ImGui's selected and hovered row fills are cleared.
            const Colour clear{ .set = true, .r = 0, .g = 0, .b = 0, .a = 0 };
            PushColour(ImGuiCol_Header, clear);
            PushColour(ImGuiCol_HeaderHovered, clear);
            PushColour(ImGuiCol_HeaderActive, clear);
        }
        if (!style.themeFrame) {
            const Colour clear{ .set = true, .r = 0, .g = 0, .b = 0, .a = 0 };
            PushColour(ImGuiCol_Border, clear);
            PushColour(ImGuiCol_ScrollbarBg, clear);
            PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ++pushedVars;
        }
    }

    void Pop()
    {
        if (pushedVars > 0) ImGuiMCP::PopStyleVar(pushedVars);
        if (pushedColours > 0) ImGuiMCP::PopStyleColor(pushedColours);
        pushedVars = 0;
        pushedColours = 0;
    }

    bool PushRowText(bool highlighted)
    {
        if (!style.active || !exportsOk || !highlighted || !style.textHighlighted.set) return false;
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, style.textHighlighted.Vec4());
        return true;
    }

    void PopRowText(bool pushed)
    {
        if (pushed) ImGuiMCP::PopStyleColor(1);
    }

    bool DrawRowHighlight(void* drawList, float x0, float y0, float x1, float y1)
    {
        if (!style.active || !exportsOk || !drawList || (!style.highlightBand.set && !style.highlightLine.set && !style.highlightMark.set)) return false;
        auto* list = static_cast<ImGuiMCP::ImDrawList*>(drawList);
        using namespace ImGuiMCP::ImDrawListManager;
        const float w = x1 - x0;
        const float t = style.lineThickness;
        if (style.highlightBand.set) {
            // Norden's item-list selection band (inventorylists.swf, ItemsListEntry's selectIndicator): fades in over the
            // first fifth, full to the middle, then fades out to nothing at the right.
            const auto& c = style.highlightBand;
            const auto full = U32(c), clear = U32(c, 0.0f);
            AddRectFilledMultiColor(list, ImGuiMCP::ImVec2{ x0, y0 }, ImGuiMCP::ImVec2{ x0 + w * 0.2f, y1 }, clear, full, full, clear);
            AddRectFilled(list, ImGuiMCP::ImVec2{ x0 + w * 0.2f, y0 }, ImGuiMCP::ImVec2{ x0 + w * 0.5f, y1 }, full, 0.0f, 0);
            AddRectFilledMultiColor(list, ImGuiMCP::ImVec2{ x0 + w * 0.5f, y0 }, ImGuiMCP::ImVec2{ x1, y1 }, full, clear, clear, full);
        }
        if (style.highlightLine.set) {
            // Its lines along the top and bottom edge: strongest at the left, gone at the right (the bottom one a little fainter).
            const auto& c = style.highlightLine;
            AddRectFilledMultiColor(list, ImGuiMCP::ImVec2{ x0, y0 }, ImGuiMCP::ImVec2{ x1, y0 + t }, U32(c), U32(c, 0.0f), U32(c, 0.0f), U32(c));
            AddRectFilledMultiColor(list, ImGuiMCP::ImVec2{ x0, y1 - t }, ImGuiMCP::ImVec2{ x1, y1 }, U32(c, 0.86f), U32(c, 0.0f), U32(c, 0.0f), U32(c, 0.86f));
        }
        if (style.highlightMark.set) {
            // And its upright mark just inside the left end, over the middle two thirds of the row's height.
            const float inset = (y1 - y0) / 7.0f;
            AddRectFilled(list, ImGuiMCP::ImVec2{ x0 + t, y0 + inset }, ImGuiMCP::ImVec2{ x0 + 2.0f * t, y1 - inset }, U32(style.highlightMark), 0.0f, 0);
        }
        return true;
    }

    bool DrawPanelEdge(void* drawList, float x0, float y0, float x1, float y1)
    {
        if (!style.active || !exportsOk || style.themeFrame || !drawList) return false;
        if (style.line.set) {
            auto* list = static_cast<ImGuiMCP::ImDrawList*>(drawList);
            FadedLine(list, x0, x1, y0, style.lineThickness, style.line, 1.0f);
            FadedLine(list, x0, x1, y1 - style.lineThickness, style.lineThickness, style.line, 1.0f);
        }
        return true;
    }

    bool DrawSeparator()
    {
        // ImGui's Separator goes through the menu framework's art hook, which draws the theme's own section rule (a
        // dotted strip under the Skyrim theme) tinted with the separator colour - not the style's line. So the style
        // draws its line itself, at the cursor, as wide as the content, and takes the same height Separator would.
        if (!style.active || !exportsOk || !style.line.set) return false;
        using namespace ImGuiMCP;
        const ImVec2 at = GetCursorScreenPos();
        const ImVec2 room = GetContentRegionAvail();
        auto* list = GetWindowDrawList();
        if (!list || room.x <= 0.0f) return false;
        FadedLine(list, at.x, at.x + room.x, at.y, style.lineThickness, style.line, 1.0f);
        Dummy(ImVec2{ 0.0f, style.lineThickness });
        return true;
    }

    std::string StateJson()
    {
        if (!style.active) return R"({"active":false})";
        std::string name;   // JSON-safe copy of sName
        for (const char c : style.name) {
            if (c == '"' || c == '\\') name += '\\';
            if (static_cast<unsigned char>(c) >= 0x20) name += c;
        }
        return fmt::format(R"({{"active":true,"name":"{}","themeFrame":{},"exportsOk":{}}})", name, style.themeFrame,
            exportsChecked ? (exportsOk ? "true" : "false") : "\"unchecked\"");
    }
}
