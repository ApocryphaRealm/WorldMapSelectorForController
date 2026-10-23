#include "Diagnostics.h"
#include "MapMenuEvents.h"
#include "MapMenuKeyHint.h"
#include "MapPanel.h"
#include "MapSelection.h"
#include "MapSwitchFlow.h"
#include "WorldspaceOverride.h"

namespace WMS::MapMenuEvents
{
    namespace
    {
        // Inheritance makes this object an event sink Skyrim can call.
        // final prevents another class from deriving from our sink.
        class MenuEventSink final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
        {
            public:
                // override asks the compiler to verify that this exactly matches
                // the virtual callback declared by BSTEventSink.
                RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
                {
                    if (!event || event->menuName != RE::MapMenu::MENU_NAME) {
                        return RE::BSEventNotifyControl::kContinue;
                    }

                    SKSE::log::info("MapMenu {}", event->opening ? "opened" : "closed");

                    if (event->opening) {
                        // MapMenu construction has already asked the resolver for its worldspace.
                        // Freeze that decision for this menu instance.
                        WorldspaceOverride::BeginSession();
                        Diagnostics::LogWorldspaceState();
                        MapMenuKeyHint::Show();
                    } else {
                        WorldspaceOverride::ResetSession();
                        MapPanel::OnMapMenuClosed();

                        // A map switch deliberately closes and reopens MapMenu.
                        // Do not consume a one-shot selection during that handoff.
                        if (!MapSwitchFlow::OnMapMenuClosed()) {
                            MapSelection::OnMapClosed();
                        }
                    }

                    // kContinue allows other registered listeners to receive the event.
                    return RE::BSEventNotifyControl::kContinue;
                }
        };
    }

    // Register the object Skyrim will notify whenever MapMenu opens or closes.
    void Register(RE::UI* ui)
    {
        // A function-local static is constructed once and lives until the DLL unloads.
        // Skyrim therefore never retains a pointer to a dead sink.
        static MenuEventSink menuEventSink;

        // & obtains the sink object's address because AddEventSink expects a pointer.
        ui->AddEventSink<RE::MenuOpenCloseEvent>(&menuEventSink);
        SKSE::log::info("Registered MapMenu event listener.");
    }
}
