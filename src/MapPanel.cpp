// The map list panel (see MapPanel.h). Built on the menu framework's AddWindow surface.
//
// Threads: Open/Dismiss/OnMapMenuClosed run on the game's main thread (input sink, menu events, tasks). Render runs on
// the render thread inside the framework's ImGui frame. The rows are snapshotted on the main thread when the panel
// opens; a choice made on the render thread is handed back to the main thread through SKSE's task interface.

#define NOMINMAX
#include "SKSEMenuFramework.h"

#include "Config.h"
#include "MapChooser.h"
#include "MapChooserInput.h"
#include "MapPanel.h"
#include "MapSelection.h"
#include "MapSwitchFlow.h"
#include "Strings.h"
#include "WorldspaceCatalog.h"

namespace WMS::MapPanel
{
    namespace
    {
        enum class RowType { kClearSelection, kMap, kFake };

        struct Row
        {
            RowType type = RowType::kMap;
            RE::TESWorldSpace* worldspace = nullptr;   // borrowed; the game owns it for the session
            std::string label;                         // what the row shows (name plus [Here] / [Selected])
            std::string displayName;                   // the map's name, for the HUD message
            bool isCurrent = false;                    // the map the menu is showing now
        };

        // The panel's life: closed; open (drawn, taking input); draining (a choice or a close was made - an invisible
        // window keeps the input until the button that did it is let go, so its release cannot reach the map).
        enum class State : int { kClosed = 0, kOpen = 1, kDraining = 2 };

        SKSEMenuFramework::Model::WindowInterface* window = nullptr;
        std::atomic<int> state { static_cast<int>(State::kClosed) };

        std::mutex rowsLock;
        std::vector<Row> rows;          // guarded by rowsLock
        int highlight = 0;              // render thread only
        int openHighlight = -1;         // guarded by rowsLock: the row Open wants highlighted (taken by the next frame)
        bool scrollToHighlight = false; // render thread only
        int drainFrames = 0;            // render thread

        std::atomic<int> pendingNavigate { 0 };
        std::atomic<bool> pendingSelect { false };
        std::atomic<int> fakeEntryCount { 0 };

        // Last drawn frame, for the DevBench state.
        std::atomic<float> lastX { 0 }, lastY { 0 }, lastW { 0 }, lastH { 0 };
        std::atomic<float> lastScrollY { 0 }, lastScrollMaxY { 0 };
        std::atomic<int> lastHighlight { 0 }, lastRowCount { 0 };
        std::atomic<std::uint32_t> opens { 0 }, selections { 0 };

        // Every export Render and the settings page call. The framework header does not null-check, so a missing
        // export would be a jump to address zero (past-mistakes library 3876): refuse the panel instead.
        bool HasRequiredExports()
        {
            constexpr const char* required[] = {
                "AddWindow", "igBegin", "igEnd", "igBeginChild_Str", "igEndChild", "igSelectable_Bool", "igSetScrollHereY",
                "igSetNextWindowPos", "igSetNextWindowSize", "igGetMainViewport", "igGetForegroundDrawList_Nil", "igGetIO",
                "RegisterInpoutEvent", "igIsMouseDown_Nil", "igGetStyle", "igPushStyleColor_Vec4", "igPopStyleColor", "igTextWrappedV", "igSetWindowFontScale", "igIsItemHovered",
                "igTextV", "igTextDisabledV", "igSeparator", "igGetScrollY", "igGetScrollMaxY", "igGetFrameHeightWithSpacing",
                "igGetTextLineHeightWithSpacing"
            };
            for (const char* name : required) {
                if (!GetMenuFrameworkFunction<void*>(name)) {
                    SKSE::log::warn("The menu framework does not export \"{}\"; the map list panel is off and the message-box chooser is used.", name);
                    return false;
                }
            }
            return true;
        }

        using DrawThemeFrameFn = bool (*)(void*, float, float, float, float);

        void RunOnMainThread(std::function<void()> task)
        {
            if (auto* tasks = SKSE::GetTaskInterface()) {
                tasks->AddTask(std::move(task));
            } else {
                SKSE::log::error("Map list panel: no SKSE task interface; the choice was not applied.");
            }
        }

        // Main thread. The upstream's catalogue and order (the map shown now first, the selected one next, then by
        // name) with [Clear Selection] on top, as the message-box chooser lists them.
        std::vector<Row> BuildRows(int& initialHighlight)
        {
            auto* currentMap  = WorldspaceCatalog::GetMapOwner(MapChooser::CurrentWorldspace());
            auto* selectedMap = WorldspaceCatalog::GetMapOwner(MapSelection::GetSelectedWorldspace());
            const auto options = WorldspaceCatalog::GetOrderedOptions(currentMap, selectedMap);

            std::vector<Row> built;
            if (Config::GetShowClearSelectionButton()) {
                built.push_back({ .type = RowType::kClearSelection, .label = TR("WMSFC_ClearSelection", "[Clear Selection]") });
            }

            initialHighlight = -1;
            int selectedRow = -1;
            for (const auto& option : options) {
                auto label = option.displayName;
                if (option.hasDuplicateDisplayName && !option.editorID.empty()) {
                    label += fmt::format(" ({})", option.editorID);
                }
                const bool isHere = option.worldspace == currentMap;
                const bool isSelected = option.worldspace == selectedMap;
                if (isHere && isSelected) label += " " + TR("WMSFC_TagHereSelected", "[Here/Selected]");
                else if (isHere) label += " " + TR("WMSFC_TagHere", "[Here]");
                else if (isSelected) label += " " + TR("WMSFC_TagSelected", "[Selected]");

                if (isSelected) selectedRow = static_cast<int>(built.size());
                if (isHere && initialHighlight < 0) initialHighlight = static_cast<int>(built.size());
                built.push_back({ .type = RowType::kMap, .worldspace = option.worldspace, .label = std::move(label),
                                  .displayName = option.displayName, .isCurrent = isHere });
            }

            if (selectedRow >= 0) initialHighlight = selectedRow;

            // Test-only rows (DevBench op=fake) that prove the list scrolls when more maps exist than fit.
            const int fakes = fakeEntryCount.load();
            for (int i = 1; i <= fakes; ++i) {
                built.push_back({ .type = RowType::kFake, .label = fmt::format("Test entry {}", i), .displayName = fmt::format("Test entry {}", i) });
            }

            if (initialHighlight < 0) initialHighlight = built.empty() ? 0 : (std::min)(static_cast<int>(built.size()) - 1, Config::GetShowClearSelectionButton() ? 1 : 0);
            return built;
        }

        // Main thread: what a chosen row does.
        void ApplyRow(const Row& row)
        {
            switch (row.type) {
                case RowType::kClearSelection:
                    MapSelection::SelectDefault();
                    RE::SendHUDMessage::ShowHUDMessage(TR("WMSFC_SelectionCleared", "World map selection cleared.").c_str());
                    SKSE::log::info("Map list panel cleared the map selection.");
                    MapSwitchFlow::Cancel();
                    break;
                case RowType::kMap: {
                    if (row.isCurrent) {
                        // Already on screen: remember it as the choice, no close-and-reopen.
                        MapSelection::Select(row.worldspace);
                        SKSE::log::info("Map list panel chose {}, the map already showing.", row.displayName);
                        MapSwitchFlow::Cancel();
                        break;
                    }
                    MapSelection::Select(row.worldspace);
                    std::string message;
                    try {
                        message = fmt::format(fmt::runtime(TR("WMSFC_MapSelected", "World map selected: {}")), row.displayName);
                    } catch (const fmt::format_error&) {
                        message = fmt::format("World map selected: {}", row.displayName);
                    }
                    RE::SendHUDMessage::ShowHUDMessage(message.c_str());
                    SKSE::log::info("Map list panel selected {} ({:08X}).", row.displayName, row.worldspace ? row.worldspace->GetFormID() : 0);
                    MapSwitchFlow::ConfigureInsideMap();
                    MapSwitchFlow::ApplyAfterChoice();
                    break;
                }
                case RowType::kFake:
                    SKSE::log::info("Map list panel: test row \"{}\" chosen (does nothing).", row.label);
                    break;
            }
        }

        // THE PANEL'S KEYS AND BUTTONS come through the menu framework's input callback (SMF's RegisterInpoutEvent),
        // which sees every game input event first - before the framework's own ImGui feed. Read from ImGui instead, the
        // first controller press after any keyboard or mouse use was lost: the framework spends it switching its input
        // mode (seen in game 2026-10-23, B did nothing once after the settings page had been used with the mouse). The
        // callback claims (returns true for) the presses it handles while the panel is open; the mouse stays with ImGui.
        enum : std::uint32_t { kHeldConfirm = 1, kHeldCancel = 2 };
        std::atomic<std::uint32_t> heldButtons { 0 };   // A/Enter or B/Esc held since they acted, for the drain
        std::atomic<bool> pendingCancel { false };
        std::atomic<bool> lastDeviceGamepad { false };

        enum class Key { kNone, kUp, kDown, kConfirm, kCancel };

        Key Classify(const RE::ButtonEvent& button)
        {
            const auto code = button.GetIDCode();
            if (button.GetDevice() == RE::INPUT_DEVICE::kGamepad) {
                switch (code) {
                    case 0x0001: return Key::kUp;        // D-pad up
                    case 0x0002: return Key::kDown;      // D-pad down
                    case 0x1000: return Key::kConfirm;   // A
                    case 0x2000: return Key::kCancel;    // B
                    default: return Key::kNone;
                }
            }
            if (button.GetDevice() == RE::INPUT_DEVICE::kKeyboard) {
                switch (code) {
                    case 0xC8: case 0x11: return Key::kUp;                    // Up arrow, W
                    case 0xD0: case 0x1F: return Key::kDown;                  // Down arrow, S
                    case 0x1C: case 0x9C: case 0x39: return Key::kConfirm;    // Enter, keypad Enter, Space
                    case 0x01: case 0x0F: case 0x0E: return Key::kCancel;     // Escape, Tab, Backspace
                    default: return Key::kNone;
                }
            }
            return Key::kNone;
        }

        // Held direction repeats: first repeat after 0.35 s, then every 0.08 s (the game reports how long it is held).
        bool RepeatDue(float heldSecs, float& lastRepeatAt)
        {
            if (heldSecs < 0.35f) return false;
            if (heldSecs - lastRepeatAt < 0.08f) return false;
            lastRepeatAt = heldSecs;
            return true;
        }

        bool __stdcall OnInputEvent(RE::InputEvent* event)
        {
            if (!event || static_cast<State>(state.load()) == State::kClosed) return false;

            if (event->GetEventType() == RE::INPUT_EVENT_TYPE::kThumbstick) {
                const auto* stick = static_cast<const RE::ThumbstickEvent*>(event);
                if (!stick->IsLeft() || static_cast<State>(state.load()) != State::kOpen) return false;
                // One row per push past half way, repeating while held (the event carries no hold time, so count time here).
                static int lastDir = 0;
                static std::chrono::steady_clock::time_point pushedAt, lastStep;
                const int dir = stick->yValue > 0.5f ? -1 : (stick->yValue < -0.5f ? 1 : 0);
                const auto now = std::chrono::steady_clock::now();
                if (dir != 0) lastDeviceGamepad = true;
                if (dir != lastDir) {
                    lastDir = dir;
                    if (dir != 0) { pendingNavigate += dir; pushedAt = lastStep = now; }
                } else if (dir != 0 && now - pushedAt > std::chrono::milliseconds(350) && now - lastStep > std::chrono::milliseconds(80)) {
                    pendingNavigate += dir;
                    lastStep = now;
                }
                return true;
            }

            const auto* button = event->AsButtonEvent();
            if (!button) return false;
            const Key key = Classify(*button);
            if (key == Key::kNone) return false;

            if (button->IsUp()) {
                if (key == Key::kConfirm) heldButtons &= ~static_cast<std::uint32_t>(kHeldConfirm);
                if (key == Key::kCancel) heldButtons &= ~static_cast<std::uint32_t>(kHeldCancel);
                return true;
            }
            if (static_cast<State>(state.load()) != State::kOpen) return true;   // draining: swallow, nothing acts

            lastDeviceGamepad = button->GetDevice() == RE::INPUT_DEVICE::kGamepad;
            static float lastRepeatAt = 0.0f;
            const bool fresh = button->IsDown();
            if (fresh) lastRepeatAt = 0.0f;
            switch (key) {
                case Key::kUp:
                    if (fresh || RepeatDue(button->HeldDuration(), lastRepeatAt)) pendingNavigate -= 1;
                    break;
                case Key::kDown:
                    if (fresh || RepeatDue(button->HeldDuration(), lastRepeatAt)) pendingNavigate += 1;
                    break;
                case Key::kConfirm:
                    if (fresh) { heldButtons |= kHeldConfirm; pendingSelect = true; }
                    break;
                case Key::kCancel:
                    if (fresh) { heldButtons |= kHeldCancel; pendingCancel = true; }
                    break;
                default:
                    break;
            }
            return true;
        }

        bool AnyReleaseOutstanding()
        {
            return heldButtons.load() != 0 || ImGuiMCP::IsMouseDown(ImGuiMCP::ImGuiMouseButton_Left);
        }

        void BeginDrain(const char* why)
        {
            drainFrames = 0;
            state = static_cast<int>(State::kDraining);
            SKSE::log::debug("Map list panel closing ({}).", why);
        }

        void FinishClose()
        {
            state = static_cast<int>(State::kClosed);
            if (window) window->IsOpen = false;
            SKSE::log::debug("Map list panel closed; input handed back.");
        }

        struct Geometry { float x, y, w, h; };

        Geometry ComputeGeometry()
        {
            const auto* viewport = ImGuiMCP::GetMainViewport();
            const float screenW = viewport ? viewport->Size.x : 1920.0f;
            const float screenH = viewport ? viewport->Size.y : 1080.0f;
            const float vx = viewport ? viewport->Pos.x : 0.0f;
            const float vy = viewport ? viewport->Pos.y : 0.0f;
            // The world map's visible area: below the map's top edge and above its button bar.
            const float top = screenH * 0.08f;
            const float bottom = screenH * 0.12f;
            const float side = screenW * 0.015f;
            const float width = screenW * static_cast<float>(Config::GetPanelWidthPercent()) / 100.0f;
            const float x = Config::GetPanelSide() == Config::PanelSide::kLeft ? vx + side : vx + screenW - side - width;
            return { x, vy + top, width, screenH - top - bottom };
        }

        void RenderDrain(const Geometry& g)
        {
            using namespace ImGuiMCP;
            // An invisible window that still takes the mouse, so the framework keeps the input until the press is let go.
            SetNextWindowPos(ImVec2{ g.x, g.y }, ImGuiCond_Always);
            SetNextWindowSize(ImVec2{ g.w, g.h }, ImGuiCond_Always);
            if (Begin("##WMSFCMapPanelDrain", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
                      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav)) {
            }
            End();
            if (!AnyReleaseOutstanding() || ++drainFrames > 30) {
                FinishClose();
            }
        }

        void __stdcall Render()
        {
            using namespace ImGuiMCP;
            const auto current = static_cast<State>(state.load());
            if (current == State::kClosed) {
                if (window) window->IsOpen = false;
                return;
            }

            const Geometry g = ComputeGeometry();
            if (current == State::kDraining) {
                RenderDrain(g);
                return;
            }

            std::vector<Row> snapshot;
            {
                std::scoped_lock lock(rowsLock);
                snapshot = rows;
                if (openHighlight >= 0) {
                    highlight = openHighlight;
                    openHighlight = -1;
                    scrollToHighlight = true;
                }
            }
            const int count = static_cast<int>(snapshot.size());
            highlight = count > 0 ? std::clamp(highlight, 0, count - 1) : 0;

            // Input: our own highlight, not ImGui's navigation (the window is NoNav), so one press moves one row. The keys and
            // buttons arrive through OnInputEvent; the mouse (hover, click, wheel) through ImGui below.
            const int move = pendingNavigate.exchange(0);
            if (move != 0 && count > 0) {
                highlight = std::clamp(highlight + move, 0, count - 1);
                scrollToHighlight = true;
            }
            int chosen = -1;
            if (pendingSelect.exchange(false)) {
                chosen = highlight;
            }
            const bool cancel = pendingCancel.exchange(false);

            SetNextWindowPos(ImVec2{ g.x, g.y }, ImGuiCond_Always);
            SetNextWindowSize(ImVec2{ g.w, g.h }, ImGuiCond_Always);
            if (Begin("##WMSFCMapPanel", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav)) {
                SetWindowFontScale(static_cast<float>(Config::GetPanelTextScalePercent()) / 100.0f);

                Text("%s", TR("WMSFC_PanelTitle", "World Maps").c_str());
                Separator();

                // The list fills the panel between the title and the hint line, and scrolls (wheel, or following the highlight).
                const float footer = GetFrameHeightWithSpacing() + 2.0f * GetTextLineHeightWithSpacing();
                if (BeginChild("##WMSFCMapRows", ImVec2{ 0.0f, -footer }, 0, ImGuiWindowFlags_NoNav)) {
                    SetWindowFontScale(static_cast<float>(Config::GetPanelTextScalePercent()) / 100.0f);
                    const auto* io = GetIO();
                    const bool mouseMoved = io && (io->MouseDelta.x != 0.0f || io->MouseDelta.y != 0.0f);
                    for (int i = 0; i < count; ++i) {
                        const auto& row = snapshot[i];
                        const auto id = fmt::format("{}##row{}", row.label, i);
                        if (Selectable(id.c_str(), i == highlight)) {
                            highlight = i;
                            chosen = i;
                        }
                        if (IsItemHovered() && mouseMoved) {
                            highlight = i;
                        }
                        if (i == highlight && scrollToHighlight) {
                            SetScrollHereY(0.5f);
                        }
                    }
                    scrollToHighlight = false;
                    lastScrollY = GetScrollY();
                    lastScrollMaxY = GetScrollMaxY();
                }
                EndChild();

                Separator();
                // The hint follows the device last used on the panel (or the one that opened it), wrapped to the panel's width.
                const auto* style = GetStyle();
                if (style) PushStyleColor(ImGuiCol_Text, style->Colors[ImGuiCol_TextDisabled]);
                TextWrapped("%s", lastDeviceGamepad ? TR("WMSFC_PanelHintPad", "D-pad: move   A: select   B: close").c_str()
                                                    : TR("WMSFC_PanelHintKeys", "Arrows or wheel: move   Enter or click: select   Esc: close").c_str());
                if (style) PopStyleColor(1);
            }
            End();

            // The active theme's frame around the panel (Nordic knotwork under the Skyrim theme); nothing under a theme without one.
            static DrawThemeFrameFn drawThemeFrame = GetMenuFrameworkFunction<DrawThemeFrameFn>("AMF_DrawThemeFrame");
            if (drawThemeFrame) {
                drawThemeFrame(GetForegroundDrawList(), g.x, g.y, g.x + g.w, g.y + g.h);
            }

            lastX = g.x; lastY = g.y; lastW = g.w; lastH = g.h;
            lastHighlight = highlight;
            lastRowCount = count;

            if (chosen >= 0 && chosen < count) {
                const Row row = snapshot[chosen];
                ++selections;
                RunOnMainThread([row] { ApplyRow(row); });
                BeginDrain("a row was chosen");
            } else if (cancel) {
                RunOnMainThread([] { MapSwitchFlow::Cancel(); });
                BeginDrain("closed without a choice");
            }
        }
    }

    bool Register()
    {
        if (window) return true;
        if (!SKSEMenuFramework::IsInstalled() || !GetMenuFrameworkModule()) {
            SKSE::log::info("No menu framework is loaded; the map chooser is the message box. Install the Apocrypha Menu Framework for the map list panel.");
            return false;
        }
        if (!HasRequiredExports()) {
            return false;
        }
        // BlockUserInput: while open, the framework takes the controller, keyboard and mouse from the game (the map stays still).
        window = SKSEMenuFramework::AddWindow(Render, true);
        // The panel's keys and buttons (see OnInputEvent). Kept for the life of the process.
        static auto* inputEvent = SKSEMenuFramework::AddInputEvent(OnInputEvent);
        (void)inputEvent;
        if (!window) {
            SKSE::log::warn("The menu framework refused the map list panel window; the message-box chooser is used.");
            return false;
        }
        window->IsOpen = false;
        SKSE::log::info("Registered the map list panel with the menu framework (side {}, width {}%, text {}%).",
            Config::GetPanelSide() == Config::PanelSide::kLeft ? "left" : "right", Config::GetPanelWidthPercent(), Config::GetPanelTextScalePercent());
        return true;
    }

    bool IsAvailable() { return window != nullptr; }

    void Open()
    {
        if (!window) return;
        auto* ui = RE::UI::GetSingleton();
        if (!ui || !ui->IsMenuOpen(RE::MapMenu::MENU_NAME)) {
            SKSE::log::debug("Ignored map list panel request because MapMenu is not open.");
            return;
        }
        if (ui->IsMenuOpen(RE::MessageBoxMenu::MENU_NAME)) {
            SKSE::log::trace("Ignored map list panel request because a message box is open.");
            return;
        }
        if (IsOpen()) return;

        int initial = 0;
        auto built = BuildRows(initial);
        const auto count = built.size();
        {
            std::scoped_lock lock(rowsLock);
            rows = std::move(built);
            openHighlight = initial;
        }
        pendingNavigate = 0;
        pendingSelect = false;
        pendingCancel = false;
        heldButtons = 0;
        lastDeviceGamepad = MapChooserInput::LastInputWasGamepad();
        ++opens;
        state = static_cast<int>(State::kOpen);
        window->IsOpen = true;
        SKSE::log::info("Map list panel opened with {} rows (highlight {}).", count, initial);
    }

    bool Dismiss()
    {
        if (static_cast<State>(state.load()) != State::kOpen) return false;
        // The render thread finishes the close (drain) on its next frame.
        state = static_cast<int>(State::kDraining);
        MapSwitchFlow::Cancel();
        return true;
    }

    bool IsOpen() { return static_cast<State>(state.load()) != State::kClosed; }

    void OnMapMenuClosed()
    {
        if (!IsOpen()) return;
        state = static_cast<int>(State::kClosed);
        if (window) window->IsOpen = false;
        SKSE::log::debug("MapMenu closed; the map list panel closed with it.");
    }

    void QueueNavigate(int rowsToMove) { pendingNavigate += rowsToMove; }
    void QueueSelect() { pendingSelect = true; }
    void SetFakeEntryCount(int count)
    {
        fakeEntryCount = std::clamp(count, 0, 200);
        SKSE::log::info("Map list panel: {} test rows will be added the next time it opens (DevBench).", fakeEntryCount.load());
    }

    std::string StateJson()
    {
        return fmt::format(
            R"({{"available":{},"open":{},"state":{},"rows":{},"highlight":{},"fakeRows":{},"scrollY":{:.1f},"scrollMaxY":{:.1f},)"
            R"("rect":[{:.0f},{:.0f},{:.0f},{:.0f}],"side":"{}","widthPercent":{},"textScalePercent":{},"opens":{},"selections":{}}})",
            window != nullptr, IsOpen(), state.load(), lastRowCount.load(), lastHighlight.load(), fakeEntryCount.load(),
            lastScrollY.load(), lastScrollMaxY.load(), lastX.load(), lastY.load(), lastW.load(), lastH.load(),
            Config::GetPanelSide() == Config::PanelSide::kLeft ? "left" : "right", Config::GetPanelWidthPercent(),
            Config::GetPanelTextScalePercent(), opens.load(), selections.load());
    }
}
