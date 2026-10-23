#include "DevBenchTool.h"

#include "Config.h"
#include "DevBench/DevBenchAPI.h"
#include "MapChooser.h"
#include "MapChooserInput.h"
#include "MapPanel.h"
#include "SettingsPage.h"
#include "Strings.h"

// DevBench tool "wmsfc.control" (rules 31 and 64): the map list panel's and the chooser's live state, and ops that drive them.
//   {}                                  -> state (panel geometry, rows, highlight, scroll, settings, counters)
//   {"op":"open"}                       -> asks for the panel (or, without the menu framework, the message box) through the
//                                          same MapMenu-only gate a key or button press uses
//   {"op":"dismiss"}                    -> closes the panel / message box without a choice
//   {"op":"nav","rows":N}               -> moves the panel's highlight N rows (negative = up)
//   {"op":"select"}                     -> chooses the highlighted row
//   {"op":"fake","count":N}             -> TEST ONLY: N extra rows the next time the panel opens, to prove it scrolls (0 = none)
//   {"op":"settings","side":"left|right","width":W,"text":T} -> the settings page's setters, then a save to the INI
// A real test still presses the real key / controller button; these prove the gate and the panel, not the input path.
// The handler runs on DevBench's listener thread, so game work is queued to the main thread.
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

        // The string value of "key" in a flat JSON object, or empty.
        std::string StringField(std::string_view args, std::string_view key)
        {
            const auto at = args.find(fmt::format("\"{}\"", key));
            if (at == std::string_view::npos) return {};
            auto open = args.find('"', args.find(':', at) + 1);
            if (open == std::string_view::npos) return {};
            const auto close = args.find('"', open + 1);
            if (close == std::string_view::npos) return {};
            return std::string(args.substr(open + 1, close - open - 1));
        }

        // The integer value of "key", or fallback.
        int IntField(std::string_view args, std::string_view key, int fallback)
        {
            const auto at = args.find(fmt::format("\"{}\"", key));
            if (at == std::string_view::npos) return fallback;
            auto pos = args.find(':', at);
            if (pos == std::string_view::npos) return fallback;
            ++pos;
            while (pos < args.size() && (args[pos] == ' ' || args[pos] == '"')) ++pos;
            int value = 0;
            const auto result = std::from_chars(args.data() + pos, args.data() + args.size(), value);
            return result.ec == std::errc{} ? value : fallback;
        }

        void ControlTool(void*, const char* argsJson, void* sink, DevBenchAPI::WriteFn write)
        {
            if (!write) return;
            const std::string_view args = argsJson ? argsJson : "";
            const auto op = StringField(args, "op");

            if (op == "open") {
                Queue([] {
                    if (MapPanel::IsAvailable()) MapPanel::Open(); else MapChooser::Open();
                });
                write(sink, R"({"ok":true,"op":"open","queued":true})");
                return;
            }
            if (op == "dismiss") {
                Queue([] {
                    if (!MapPanel::Dismiss()) MapChooser::Dismiss();
                });
                write(sink, R"({"ok":true,"op":"dismiss","queued":true})");
                return;
            }
            if (op == "nav") {
                const int rows = IntField(args, "rows", 1);
                MapPanel::QueueNavigate(rows);
                write(sink, fmt::format(R"({{"ok":true,"op":"nav","rows":{}}})", rows).c_str());
                return;
            }
            if (op == "select") {
                MapPanel::QueueSelect();
                write(sink, R"({"ok":true,"op":"select","queued":true})");
                return;
            }
            if (op == "fake") {
                const int count = IntField(args, "count", 0);
                MapPanel::SetFakeEntryCount(count);
                write(sink, fmt::format(R"({{"ok":true,"op":"fake","count":{},"note":"applies the next time the panel opens"}})", count).c_str());
                return;
            }
            if (op == "settings") {
                SettingsPage::ApplyFromTool(StringField(args, "side"), IntField(args, "width", 0), IntField(args, "text", 0));
                write(sink, R"({"ok":true,"op":"settings","saved":true})");
                return;
            }

            auto* ui = RE::UI::GetSingleton();
            const bool mapOpen = ui && ui->IsMenuOpen(RE::MapMenu::MENU_NAME);
            const bool boxOpen = ui && ui->IsMenuOpen(RE::MessageBoxMenu::MENU_NAME);
            const auto counters = MapChooserInput::GetCounters();
            const auto json = fmt::format(
                R"({{"ok":true,"mapMenuOpen":{},"messageBoxOpen":{},"chooserActive":{},"panel":{},"mapMenuKey":{},"mapMenuButton":{},)"
                R"("lastInputWasGamepad":{},"opensFromKeyboard":{},"opensFromGamepad":{},"ignoredOutsideMap":{},"language":"{}"}})",
                mapOpen, boxOpen, MapChooser::IsActive(), MapPanel::StateJson(), Config::GetMapMenuKey(), Config::GetMapMenuButton(),
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
            "\"description\":\"World Map Selector for Controller: the map list panel (open, rows, highlight, scroll, rectangle, side, "
            "width, text size) and the chooser's state. op=open asks for the panel through the same MapMenu-only gate a press uses; "
            "op=dismiss closes it; op=nav rows=N moves the highlight; op=select chooses it; op=fake count=N adds N test rows the next "
            "time it opens (test only); op=settings side/width/text sets and saves the page's settings.\","
            "\"inputSchema\":{\"type\":\"object\",\"properties\":{\"op\":{\"type\":\"string\",\"enum\":[\"state\",\"open\",\"dismiss\",\"nav\",\"select\",\"fake\",\"settings\"]},"
            "\"rows\":{\"type\":\"integer\"},\"count\":{\"type\":\"integer\"},\"side\":{\"type\":\"string\",\"enum\":[\"left\",\"right\"]},"
            "\"width\":{\"type\":\"integer\"},\"text\":{\"type\":\"integer\"}}},"
            "\"readOnly\":false"
            "}";

        devBench->RegisterTool("wmsfc.control", descriptor, &ControlTool, nullptr);
        registered = true;
        SKSE::log::info("Registered \"wmsfc.control\" with DevBench/TestBench (build {}).", devBench->GetBuildNumber());
    }
}
