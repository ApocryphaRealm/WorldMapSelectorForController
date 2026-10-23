#include "AddressLibraryGuard.h"
#include "Config.h"
#include "DevBenchTool.h"
#include "Hooks.h"
#include "Strings.h"
#include "MapChooserInput.h"
#include "MapMenuEvents.h"
#include "MapMarkerOverride.h"
#include "RoutedMarkerOverride.h"
#include "WorldspaceCatalog.h"
#include "WorldspaceOverride.h"

namespace
{
    // SKSE calls this function after each stage of game startup.
    // The message parameter points to a small object describing which stage just finished.
    void OnSKSEMessage(SKSE::MessagingInterface::Message* message)
    {
		// Return immediately if the message pointer is null.
        if (!message) { return; }

        if (message->type == SKSE::MessagingInterface::kPostLoad) {
            // DevBench/TestBench answers its interface request from kPostLoad on; retried at kDataLoaded (rule 17).
            WMS::DevBenchTool::Init(false);
        } else if (message->type == SKSE::MessagingInterface::kInputLoaded) {
            // Skyrim's input devices and input event source now exist,
            // so the chooser can safely register its keyboard listener.
            WMS::Config::ApplyFrameworkReservedKeys();
            WMS::MapChooserInput::Register();
        } else if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            // Every active plugin has finished loading and its forms have their final runtime FormIDs,
            // so it is now safe to build the map catalogue.
            WMS::WorldspaceCatalog::Build();
            // The game's language setting is final by now.
            WMS::Strings::Load();
            WMS::DevBenchTool::Init(true);
        }
    }
}

// SKSE calls this exported entry point when it loads the DLL.
// Returning true accepts the plugin; returning false tells SKSE that initialization failed.
SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    // Before SKSE::Init (which opens the Address Library itself): name the file CommonLib will open, and load
    // inert (returning true, the game continues) with a message naming it when it is missing.
    if (!AddressLibraryGuard::Guard("World Map Selector for Controller")) {
        return true;
    }

    // Save the interfaces supplied through the skse pointer,
    // and initialize CommonLib's SKSE API and plugin logger.
    SKSE::Init(skse);

    // Read all settings once for this game process.
    WMS::Config::Load();

    // The original World Map Selector hooks the same game functions. SKSE loads plugins in name order, so
    // WorldMapSelector.dll is already loaded when this one starts; stand down rather than hook twice.
    if (REX::W32::GetModuleHandleW(L"WorldMapSelector.dll")) {
        SKSE::log::error("The original World Map Selector (WorldMapSelector.dll) is also installed. Disable one of the two; "
                         "World Map Selector for Controller stays inactive this session.");
        return true;
    }

    // Resolve every interface needed after hook activation before changing any game function.
    auto* messaging = SKSE::GetMessagingInterface();
    auto* ui = RE::UI::GetSingleton();
    if (!messaging || !ui) {
        SKSE::log::error("Could not obtain the required SKSE/UI interfaces.");
        return false;
    }

    if (!WMS::Hooks::Initialize()) { return false; }

    // Create every detour in a disabled state.
    // If any creation fails, Reset removes all previously created hooks before SKSE rejects the plugin.
    if (!WMS::WorldspaceOverride::CreateHook() || !WMS::MapMarkerOverride::CreateHooks() || !WMS::RoutedMarkerOverride::CreateHooks()) {
        WMS::Hooks::Reset();
        return false;
    }

	// Enable all hooks in a single step.
    if (!WMS::Hooks::EnableAll()) {
        WMS::Hooks::Reset();
        return false;
    }

    // RegisterListener stores OnSKSEMessage as the callback SKSE will invoke.
    if (!messaging->RegisterListener(OnSKSEMessage)) {
        SKSE::log::error("Could not register the SKSE message listener.");
        WMS::Hooks::Reset();
        return false;
    }

    // Subscribe to MapMenu open/close events.
    // These events freeze and clear the selected-map session and coordinate close/reopen map switches.
    WMS::MapMenuEvents::Register(ui);

    // All required initialization above completed without returning false.
    SKSE::log::info("World Map Selector for Controller {}.{}.{} loaded successfully.", WMS_VERSION_MAJOR, WMS_VERSION_MINOR, WMS_VERSION_PATCH);
    return true;
}
