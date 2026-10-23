#define NOMINMAX
#include "SKSEMenuFramework.h"
#include "PreciseSlider.h"
#include "Toggle.h"

#include "Config.h"
#include "SettingsPage.h"
#include "Strings.h"

namespace WMS::SettingsPage
{
    namespace
    {
        std::string statusMessage;   // render thread
        bool saveWanted = false;     // render thread: a change waits for the control to be let go, then saves once

        bool HasRequiredExports()
        {
            // Every export this page (and Toggle.h / PreciseSlider.h) resolves - the header does not null-check.
            constexpr const char* required[] = {
                "AddSectionItem", "igTextV", "igTextDisabledV", "igTextWrappedV", "igSetTooltipV", "igSeparatorText",
                "igRadioButton_Bool", "igSliderInt", "igIsItemHovered", "igButton", "igSameLine", "igSpacing", "igPushItemWidth",
                "igPopItemWidth", "igIsAnyItemActive", "igIsKeyDown_Nil", "igIsMouseDown_Nil", "igGetCursorScreenPos",
                "igGetWindowDrawList", "igGetFrameHeight", "igInvisibleButton", "igPushID_Str", "igPopID",
                "ImDrawList_AddRectFilled", "ImDrawList_AddCircleFilled"
            };
            for (const char* name : required) {
                if (!GetMenuFrameworkFunction<void*>(name)) {
                    SKSE::log::warn("The menu framework does not export \"{}\"; the settings page is not registered (the INI still works).", name);
                    return false;
                }
            }
            return true;
        }

        void Help(const std::string& text)
        {
            ImGuiMCP::SameLine();
            ImGuiMCP::TextDisabled("%s", TR("WMSFC_HelpMark", "(?)").c_str());
            if (ImGuiMCP::IsItemHovered()) {
                ImGuiMCP::SetTooltip("%s", text.c_str());
            }
        }

        void SaveOnMainThread()
        {
            if (auto* tasks = SKSE::GetTaskInterface()) {
                tasks->AddTask([] { Config::Save(); });
            }
        }

        bool ToggleRow(const std::string& label, bool value, void (*setter)(bool), const std::string& help)
        {
            bool v = value;
            if (ImGuiMCP::Toggle(label.c_str(), &v)) {
                setter(v);
                Help(help);
                return true;
            }
            Help(help);
            return false;
        }

        void __stdcall Render()
        {
            using namespace ImGuiMCP;
            bool changed = false;

            TextWrapped("%s", TR("WMSFC_PageIntro", "Changes apply at once and are saved to the mod's INI.").c_str());
            Spacing();

            SeparatorText(TR("WMSFC_PagePanel", "Map list").c_str());
            Text("%s", TR("WMSFC_PageSide", "Side of the world map").c_str());
            SameLine();
            const bool left = Config::GetPanelSide() == Config::PanelSide::kLeft;
            if (RadioButton(TR("WMSFC_PageSideRight", "Right").c_str(), !left)) { Config::SetPanelSide(Config::PanelSide::kRight); changed = true; }
            SameLine();
            if (RadioButton(TR("WMSFC_PageSideLeft", "Left").c_str(), left)) { Config::SetPanelSide(Config::PanelSide::kLeft); changed = true; }
            Help(TR("WMSFC_HelpSide", "Which edge of the world map the list of maps runs down."));

            PushItemWidth(260.0f);
            int width = Config::GetPanelWidthPercent();
            if (precise::SliderInt(TR("WMSFC_PageWidth", "Width").c_str(), &width, Config::kPanelWidthMin, Config::kPanelWidthMax, "%d%%")) {
                Config::SetPanelWidthPercent(width);
                changed = true;
            }
            Help(TR("WMSFC_HelpWidth", "How wide the list is. Its height always fills the map from top to bottom."));

            int text = Config::GetPanelTextScalePercent();
            if (precise::SliderInt(TR("WMSFC_PageTextSize", "Text size").c_str(), &text, Config::kTextScaleMin, Config::kTextScaleMax, "%d%%")) {
                Config::SetPanelTextScalePercent(text);
                changed = true;
            }
            Help(TR("WMSFC_HelpTextSize", "The size of the list's text, against the menu's normal size."));
            PopItemWidth();

            SeparatorText(TR("WMSFC_PageBehaviour", "Behaviour").c_str());
            changed |= ToggleRow(TR("WMSFC_PagePersist", "Keep the chosen map after closing the map"), Config::GetPersistSelection(), &Config::SetPersistSelection,
                TR("WMSFC_HelpPersist", "On: the map you chose stays chosen. Off: it applies to one look at the map."));
            changed |= ToggleRow(TR("WMSFC_PageClear", "Show [Clear Selection] in the list"), Config::GetShowClearSelectionButton(), &Config::SetShowClearSelectionButton,
                TR("WMSFC_HelpClear", "The row that hands the choice back to the game, so the map shows the world you are in."));
            changed |= ToggleRow(TR("WMSFC_PageHint", "Show the button hint on the world map"), Config::GetShowMapMenuKeyHint(), &Config::SetShowMapMenuKeyHint,
                TR("WMSFC_HelpHint", "Adds Select Map to SkyUI's button bar. Takes effect the next time the map opens."));
            changed |= ToggleRow(TR("WMSFC_PageHintLocal", "Show the button hint on the local map too"), Config::GetShowMapMenuKeyHintOnLocalMap(), &Config::SetShowMapMenuKeyHintOnLocalMap,
                TR("WMSFC_HelpHintLocal", "Off by default, so it does not overlap hints that local-map mods add."));

            Spacing();
            if (Button(TR("WMSFC_PageRestore", "Restore defaults").c_str())) {
                Config::RestoreDefaults();
                changed = true;
                statusMessage = TR("WMSFC_StatusRestored", "Defaults restored and saved.");
            }
            Help(TR("WMSFC_HelpRestore", "Right side, the shipped width and text size, and the switches as they came."));

            if (changed) saveWanted = true;
            // One write per change: wait until no control is held (a slider being dragged changes every frame).
            if (saveWanted && !IsAnyItemActive()) {
                saveWanted = false;
                SaveOnMainThread();
                if (statusMessage.empty() || statusMessage == TR("WMSFC_StatusSaved", "Saved.")) {
                    statusMessage = TR("WMSFC_StatusSaved", "Saved.");
                }
            }
            if (!statusMessage.empty()) {
                TextDisabled("%s", statusMessage.c_str());
            }
            TextDisabled("%s", Config::GetIniPath().c_str());
        }
    }

    void Register()
    {
        if (!SKSEMenuFramework::IsInstalled() || !GetMenuFrameworkModule()) {
            SKSE::log::info("No menu framework is loaded; settings are read from the INI only.");
            return;
        }
        if (!HasRequiredExports()) {
            return;
        }
        SKSEMenuFramework::SetSection("World Map Selector for Controller");
        SKSEMenuFramework::AddSectionItem("Settings", Render);
        SKSE::log::info("Registered the settings page with the menu framework.");
    }

    void ApplyFromTool(const std::string& side, int widthPercent, int textScalePercent)
    {
        if (side == "left") Config::SetPanelSide(Config::PanelSide::kLeft);
        else if (side == "right") Config::SetPanelSide(Config::PanelSide::kRight);
        if (widthPercent > 0) Config::SetPanelWidthPercent(widthPercent);
        if (textScalePercent > 0) Config::SetPanelTextScalePercent(textScalePercent);
        Config::Save();
    }
}
