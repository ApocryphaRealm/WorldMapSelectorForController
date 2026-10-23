#include "Config.h"
#include "MapChooserInput.h"
#include "MapMenuKeyHint.h"
#include "Strings.h"

#include <bit>

namespace WMS::MapMenuKeyHint
{
    namespace
    {
        bool GetSkyUIButtonPanel(RE::GFxMovieView* movie, RE::GFxValue& buttonPanel, RE::GFxValue& buttons)
        {
            // Detect the interface actually exposed by the loaded MapMenu movie.
            // This is safer than checking for SkyUI.esp because another UI mod may replace MapMenu.swf.
            if (!movie->GetVariable(&buttonPanel, "_root.bottomBar.buttonPanel") || !buttonPanel.IsDisplayObject()) {
                return false;
            }

            return buttonPanel.GetMember("buttons", &buttons) && buttons.IsArray();
        }

        // The hint's text in the game's language. Static so the string outlives every GFxValue that points at it.
        const std::string& HintLabel()
        {
            static const std::string label = TR("WMSFC_SelectMap", "Select Map");
            return label;
        }

        bool IsButtonConfigured(const RE::GFxValue& button)
        {
            // SkyUI's clearButtons method empties each label during a platform rebuild.
            // A retained label therefore means our entry is already present and needs no work.
            RE::GFxValue label;
            return button.GetMember("label", &label) && label.IsString() &&
                   std::string_view(label.GetString()) == HintLabel();
        }

        bool CreateSkyUIButton(RE::GFxValue& buttonPanel, RE::GFxValue& buttons, RE::GFxValue& button)
        {
            // Re-register the retained clip after returning from a local map. Keeping the
            // configured clip avoids attaching another child with the same ActionScript name.
            if (buttonPanel.GetMember("worldMapSelectorButton", &button) && button.IsDisplayObject()) {
                for (std::uint32_t index = 0; index < buttons.GetArraySize(); ++index) {
                    RE::GFxValue candidate;
                    RE::GFxValue name;
                    if (buttons.GetElement(index, &candidate) && candidate.IsDisplayObject() &&
                        candidate.GetMember("_name", &name) && name.IsString() &&
                        std::string_view(name.GetString()) == "worldMapSelectorButton") {
                        return true;
                    }
                }

                if (!buttons.PushBack(button) ||
                    !buttonPanel.SetMember("maxButtons", buttons.GetArraySize()) ||
                    !button.SetMember("_visible", true) ||
                    !buttonPanel.Invoke("updateButtons", nullptr, std::array { RE::GFxValue(true) })) {
                    return false;
                }

                return true;
            }

            // Reuse SkyUI's renderer and initializer so the new button inherits the active UI skin.
            RE::GFxValue renderer;
            RE::GFxValue initializer;
            RE::GFxValue depth;
            if (!buttonPanel.GetMember("buttonRenderer", &renderer) || !renderer.IsString() ||
                !buttonPanel.GetMember("buttonInitializer", &initializer) ||
                !buttonPanel.Invoke("getNextHighestDepth", &depth) || !depth.IsNumber()) {
                return false;
            }

            // Attach one additional button clip to SkyUI's panel and register it in the public array.
            // SkyUI's addButton method can then treat it as the eighth normal panel entry.
            const RE::GFxValue* initializerArgument = initializer.IsObject() ? &initializer : nullptr;
            if (!buttonPanel.AttachMovie(
                    &button,
                    renderer.GetString(),
                    "worldMapSelectorButton",
                    static_cast<std::int32_t>(depth.GetSInt()),
                    initializerArgument) ||
                !button.IsDisplayObject() || !buttons.PushBack(button)) {
                return false;
            }

            return buttonPanel.SetMember("maxButtons", buttons.GetArraySize());
        }

        // SkyUI's button art reads SKSE key codes: keyboard scan codes as they are, controller buttons as
        // 266 + the button's index (D-pad up 266 ... Y 279, LT 280, RT 281). Returns 0 for a mask with no art.
        std::uint32_t GamepadMaskToSkseKeyCode(std::uint32_t mask)
        {
            switch (mask) {
                case 0x0009: return 280;  // left trigger
                case 0x000A: return 281;  // right trigger
                default: break;
            }
            if (mask == 0 || (mask & (mask - 1)) != 0) return 0;  // not exactly one button
            auto index = static_cast<std::uint32_t>(std::countr_zero(mask));
            if (index >= 12) index -= 2;  // XInput leaves bits 10 and 11 unused
            return index <= 13 ? 266 + index : 0;
        }

        // The key SkyUI should draw: the controller button while the controller is in use, else the keyboard key.
        std::uint32_t HintKeyCode()
        {
            if (MapChooserInput::LastInputWasGamepad()) {
                if (const auto code = GamepadMaskToSkseKeyCode(Config::GetMapMenuButton())) return code;
            }
            return Config::GetMapMenuKey();
        }

        void CreateButtonData(RE::GFxMovieView* movie, RE::GFxValue& buttonData, const std::string& label)
        {
            // Describe the trigger using the same object format as SkyUI's built-in hints.
            RE::GFxValue controls;
            movie->CreateObject(&controls);
            controls.SetMember("keyCode", HintKeyCode());

            movie->CreateObject(&buttonData);
            buttonData.SetMember("text", label.c_str());
            buttonData.SetMember("controls", controls);
        }

        bool IsLocalMapShowing(const RE::MapMenu& mapMenu)
        {
            const auto* mapData = mapMenu.GetRuntimeData();
            return mapData && mapData->localMapMenu.GetRuntimeData().showingMap;
        }

        bool RemoveSkyUIButton(RE::GFxValue& buttonPanel, RE::GFxValue& buttons)
        {
            RE::GFxValue button;
            if (!buttonPanel.GetMember("worldMapSelectorButton", &button) || !button.IsDisplayObject()) {
                return true;
            }

            // Remove our clip from SkyUI's public array so its layout slot is released. Retain
            // the configured clip itself so it can be registered again on the world map.
            for (std::uint32_t index = 0; index < buttons.GetArraySize(); ++index) {
                RE::GFxValue candidate;
                RE::GFxValue name;
                if (buttons.GetElement(index, &candidate) && candidate.IsDisplayObject() &&
                    candidate.GetMember("_name", &name) && name.IsString() &&
                    std::string_view(name.GetString()) == "worldMapSelectorButton") {
                    if (!buttons.RemoveElement(index)) {
                        return false;
                    }
                    break;
                }
            }

            const auto buttonCount = buttons.GetArraySize();
            if (!buttonPanel.SetMember("maxButtons", buttonCount)) {
                SKSE::log::warn("SkyUI key-hint removal failed while updating maxButtons to {}.", buttonCount);
                return false;
            }

            if (!button.SetMember("_visible", false)) {
                SKSE::log::warn("SkyUI key-hint removal failed while hiding the retained button clip.");
                return false;
            }

            if (!buttonPanel.Invoke("updateButtons", nullptr, std::array { RE::GFxValue(true) })) {
                SKSE::log::warn("SkyUI key-hint removal failed while updating the button layout.");
                return false;
            }

            return true;
        }
    }

    void Show()
    {
        if (!Config::GetShowMapMenuKeyHint()) {
            return;
        }

        auto* ui = RE::UI::GetSingleton();
        if (!ui) {
            SKSE::log::warn("Could not get the UI singleton while adding the MapMenu key hint.");
            return;
        }

        const auto mapMenu = ui->GetMenu<RE::MapMenu>();
        if (!mapMenu || !mapMenu->uiMovie) {
            SKSE::log::warn("MapMenu movie was unavailable while adding the key hint.");
            return;
        }

        auto* movie = mapMenu->uiMovie.get();
        RE::GFxValue buttonPanel;
        RE::GFxValue buttons;
        if (!GetSkyUIButtonPanel(movie, buttonPanel, buttons)) {
            SKSE::log::debug("MapMenu does not expose SkyUI's expected button panel; key hint was skipped.");
            return;
        }

        if (IsLocalMapShowing(*mapMenu) && !Config::GetShowMapMenuKeyHintOnLocalMap()) {
            if (!RemoveSkyUIButton(buttonPanel, buttons)) {
                SKSE::log::warn("Could not remove the SkyUI MapMenu key hint for the local map.");
            }
            return;
        }

        RE::GFxValue button;
        if (!CreateSkyUIButton(buttonPanel, buttons, button)) {
            SKSE::log::warn("Could not extend SkyUI's MapMenu button panel.");
            return;
        }

        if (IsButtonConfigured(button)) {
            return;
        }

        RE::GFxValue buttonData;
        CreateButtonData(movie, buttonData, HintLabel());

        // Add the new entry through SkyUI's normal population and layout methods.
        const auto buttonCount = buttons.GetArraySize();
        RE::GFxValue configuredButton;
        if (!buttonPanel.Invoke("addButton", &configuredButton, std::array { buttonData })) {
            SKSE::log::warn("SkyUI key-hint configuration failed while invoking addButton; panelButtons={}.", buttonCount);
            return;
        }

        if (!configuredButton.IsDisplayObject()) {
            SKSE::log::warn(
                "SkyUI addButton returned a non-display value; panelButtons={}, resultType={}.",
                buttonCount,
                static_cast<std::uint32_t>(configuredButton.GetType()));
            return;
        }

        if (!buttonPanel.Invoke("updateButtons", nullptr, std::array { RE::GFxValue(true) })) {
            SKSE::log::warn("SkyUI key-hint configuration failed while updating the layout; panelButtons={}.", buttonCount);
            return;
        }

        SKSE::log::debug("Added a separate SkyUI MapMenu key hint for SKSE key code {}.", HintKeyCode());
    }

    void RefreshAfterInputModeChange()
    {
        // Input events also arrive during gameplay. Only queue UI work when MapMenu is active.
        auto* ui = RE::UI::GetSingleton();
        if (!ui || !ui->IsMenuOpen(RE::MapMenu::MENU_NAME)) {
            return;
        }

        // Run after the current input dispatch finishes so SkyUI can rebuild its platform-specific
        // button list first. Show is idempotent and exits when the hint is already configured.
        const auto* tasks = SKSE::GetTaskInterface();
        if (!tasks) {
            SKSE::log::warn("Could not schedule the MapMenu key hint after an input-mode change.");
            return;
        }

        tasks->AddUITask(Show);
    }

    void RefreshAfterMapModeChange()
    {
        // Skyrim updates LocalMapMenu::showingMap while processing the Local Map input.
        // Queue the refresh so Show observes the new mode rather than the old one.
        const auto* tasks = SKSE::GetTaskInterface();
        if (!tasks) {
            SKSE::log::warn("Could not schedule the MapMenu key hint after a map-mode change.");
            return;
        }

        tasks->AddUITask(Show);
    }
}
