#include "DevBenchTool.h"

#include "Config.h"
#include "DevBench/DevBenchAPI.h"
#include "MapChooser.h"
#include "MapChooserInput.h"
#include "Strings.h"

// DevBench tool "wmsfc.control" (rules 31 and 64): the chooser's live state, and ops that drive it.
//   {}                 -> state
//   {"op":"open"}      -> asks for the chooser through the same gate a key or button press uses
//                         (MapChooser::Open: MapMenu must be open, no other message box)
//   {"op":"dismiss"}   -> closes the chooser if it is showing
// A real test still presses the real key / controller button; "open" proves the gate, not the input path.
// The handler runs on DevBench's listener thread, so game work is queued to the UI thread and state is read
// through thread-safe accessors only.
namespace WMS::DevBenchTool
{
    namespace
    {
        void Queue(void (*task)())
        {
            if (const auto* tasks = SKSE::GetTaskInterface()) {
                tasks->AddUITask(task);
            } else {
                SKSE::log::warn("DevBench tool: no SKSE task interface; the op was not queued.");
            }
        }

        void ControlTool(void*, const char* argsJson, void* sink, DevBenchAPI::WriteFn write)
        {
            if (!write) return;
            const std::string_view args = argsJson ? argsJson : "";

            if (args.find("\"open\"") != std::string_view::npos) {
                Queue([] { MapChooser::Open(); });
                write(sink, R"({"ok":true,"op":"open","queued":true})");
                return;
            }
            if (args.find("\"dismiss\"") != std::string_view::npos) {
                Queue([] { MapChooser::Dismiss(); });
                write(sink, R"({"ok":true,"op":"dismiss","queued":true})");
                return;
            }

            auto* ui = RE::UI::GetSingleton();
            const bool mapOpen = ui && ui->IsMenuOpen(RE::MapMenu::MENU_NAME);
            const bool boxOpen = ui && ui->IsMenuOpen(RE::MessageBoxMenu::MENU_NAME);
            const auto counters = MapChooserInput::GetCounters();
            const auto json = fmt::format(
                R"({{"ok":true,"mapMenuOpen":{},"messageBoxOpen":{},"chooserActive":{},"mapMenuKey":{},"mapMenuButton":{},)"
                R"("lastInputWasGamepad":{},"opensFromKeyboard":{},"opensFromGamepad":{},"ignoredOutsideMap":{},"language":"{}"}})",
                mapOpen, boxOpen, MapChooser::IsActive(), Config::GetMapMenuKey(), Config::GetMapMenuButton(),
                MapChooserInput::LastInputWasGamepad(), counters.opensFromKeyboard, counters.opensFromGamepad,
                counters.ignoredOutsideMap, Strings::Language());
            write(sink, json.c_str());
        }
    }

    void Init(bool lastAttempt)
    {
        static bool registered = false;
        if (registered) return;

        auto* devBench = DevBenchAPI::GetDevBenchInterface001();
        if (!devBench) {
            if (lastAttempt) {
                SKSE::log::info("DevBench/TestBench not detected; the \"wmsfc.control\" tool is not registered (normal for players).");
            } else {
                SKSE::log::debug("DevBench/TestBench not detected yet; will retry at kDataLoaded.");
            }
            return;
        }

        constexpr const char* descriptor =
            "{"
            "\"description\":\"World Map Selector for Controller: chooser state (MapMenu open, chooser showing, the trigger key and "
            "controller button, how often each opened it, presses ignored outside the map). op=open asks for the chooser through "
            "the same MapMenu-only gate a press uses; op=dismiss closes it.\","
            "\"inputSchema\":{\"type\":\"object\",\"properties\":{\"op\":{\"type\":\"string\",\"enum\":[\"state\",\"open\",\"dismiss\"]}}},"
            "\"readOnly\":false"
            "}";

        devBench->RegisterTool("wmsfc.control", descriptor, &ControlTool, nullptr);
        registered = true;
        SKSE::log::info("Registered \"wmsfc.control\" with DevBench/TestBench (build {}).", devBench->GetBuildNumber());
    }
}
