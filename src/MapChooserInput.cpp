#include "Config.h"
#include "MapChooser.h"
#include "MapChooserInput.h"
#include "MapMenuKeyHint.h"
#include "MapPanel.h"

namespace WMS::MapChooserInput
{
    namespace
    {
        // Live counters for the DevBench tool (rules 31 and 64). Written on the input thread, read on DevBench's.
        std::atomic<bool> lastInputWasGamepad { false };
        std::atomic<std::uint32_t> opensFromKeyboard { 0 };
        std::atomic<std::uint32_t> opensFromGamepad { 0 };
        std::atomic<std::uint32_t> ignoredOutsideMap { 0 };

        bool IsMapMenuOpen()
        {
            auto* ui = RE::UI::GetSingleton();
            return ui && ui->IsMenuOpen(RE::MapMenu::MENU_NAME);
        }

        // The sink receives linked lists of input events from Skyrim.
        class InputEventSink final : public RE::BSTEventSink<RE::InputEvent*>
        {
            public:
                RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* events, RE::BSTEventSource<RE::InputEvent*>*) override
                {
                    static std::optional<bool> previousGamepadMode;

                    if (!events) return RE::BSEventNotifyControl::kContinue;

                    // events points to the first pointer in Skyrim's linked list.
                    // Each event->next advances until a null pointer ends the list.
                    for (auto* event = *events; event; event = event->next) {
                        // SkyUI rebuilds its MapMenu button panel when input changes between
                        // keyboard/mouse and controller. Restore our hint (with that device's button) after that rebuild.
                        const bool usesGamepad = event->device == RE::INPUT_DEVICE::kGamepad;
                        const bool usesKeyboardOrMouse =
                            event->device == RE::INPUT_DEVICE::kKeyboard || event->device == RE::INPUT_DEVICE::kMouse;

                        if ((usesGamepad || usesKeyboardOrMouse) &&
                            (!previousGamepadMode || *previousGamepadMode != usesGamepad)) {
                            previousGamepadMode = usesGamepad;
                            lastInputWasGamepad = usesGamepad;
                            SKSE::log::debug("Input mode changed to {}.", usesGamepad ? "controller" : "keyboard/mouse");
                            MapMenuKeyHint::RefreshAfterInputModeChange();
                        }

                        // AsButtonEvent returns null when this input event is not a button.
                        const auto* button = event->AsButtonEvent();
                        if (!button || !button->IsDown()) {
                            continue;
                        }

                        // LocalMapMenu changes mode later in the same input dispatch. Refresh the
                        // SkyUI hint afterward so it follows the resulting world/local map state.
                        const auto* userEvents = RE::UserEvents::GetSingleton();
                        if (userEvents && button->GetUserEvent() == userEvents->localMap) {
                            MapMenuKeyHint::RefreshAfterMapModeChange();
                        }

                        // Skyrim maps Escape, Tab, and controller Back buttons to its logical Cancel event.
                        const bool isCancel = userEvents && button->GetUserEvent() == userEvents->cancel;

                        if (isCancel && (MapChooser::Dismiss() || MapPanel::Dismiss())) {
                            return RE::BSEventNotifyControl::kStop;
                        }

                        // While the panel is open it reads its own input through the menu framework.
                        if (MapPanel::IsOpen()) {
                            continue;
                        }

                        // One keyboard key and one controller button, read from [Controls]. 0 turns a device off.
                        const auto code = button->GetIDCode();
                        const bool isKey    = button->device == RE::INPUT_DEVICE::kKeyboard && Config::GetMapMenuKey() != 0 && code == Config::GetMapMenuKey();
                        const bool isButton = button->device == RE::INPUT_DEVICE::kGamepad && Config::GetMapMenuButton() != 0 && code == Config::GetMapMenuButton();
                        if (!isKey && !isButton) {
                            continue;
                        }

                        // The trigger belongs to the world map: anywhere else the press is left alone for the game.
                        if (!IsMapMenuOpen()) {
                            static bool loggedOutside = false;
                            ++ignoredOutsideMap;
                            if (!loggedOutside) {
                                loggedOutside = true;
                                SKSE::log::debug("Map chooser {} 0x{:X} pressed outside MapMenu; ignored (logged once).", isKey ? "key" : "button", code);
                            }
                            continue;
                        }

                        SKSE::log::debug("Map chooser {} 0x{:X} pressed in MapMenu; opening the chooser.", isKey ? "key" : "button", code);
                        if (isKey) ++opensFromKeyboard; else ++opensFromGamepad;
                        // The list panel when the menu framework is there, else the message box. Both repeat the
                        // menu-state checks (another message box, MapMenu open) before showing anything.
                        if (MapPanel::IsAvailable()) {
                            MapPanel::Open();
                        } else {
                            MapChooser::Open();
                        }
                        break;
                    }

                    return RE::BSEventNotifyControl::kContinue;
                }
        };
    }

    // Register installs the map-trigger and chooser-dismiss listener.
    bool Register()
    {
        auto* input = RE::BSInputDeviceManager::GetSingleton();
        if (!input) {
            SKSE::log::error("Could not get the input-device manager.");
            return false;
        }

        // Static lifetime keeps the registered sink alive for the whole process.
        static InputEventSink inputEventSink;
        // & passes the sink's address to Skyrim rather than copying the object.
        input->AddEventSink(&inputEventSink);

        SKSE::log::info("Registered the world-map chooser trigger: key 0x{:02X}, controller button 0x{:04X} (MapMenu only).",
            Config::GetMapMenuKey(), Config::GetMapMenuButton());
        return true;
    }

    bool LastInputWasGamepad() { return lastInputWasGamepad; }

    Counters GetCounters()
    {
        return { opensFromKeyboard.load(), opensFromGamepad.load(), ignoredOutsideMap.load() };
    }
}
