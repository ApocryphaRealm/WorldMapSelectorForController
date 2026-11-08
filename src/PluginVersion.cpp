#include "REL/Version.h"
#include "SKSE/SKSE.h"

SKSEPluginInfo(
    .Version = REL::Version{ WMS_VERSION_MAJOR, WMS_VERSION_MINOR, WMS_VERSION_PATCH, WMS_VERSION_TWEAK },
    .Name = "WorldMapSelectorForController"sv,
    .Author = "ApocryphaRealm"sv,
    .SupportEmail = ""sv,
    .StructCompatibility = SKSE::StructCompatibility::Independent,
    // 1.2.1: Address Library, not a list of runtimes. A list made SKSE refuse every runtime missing from it as
    // "incompatible" (GOG 1.6.1179, Nexus report 2026-11-08); every address here is an Address Library ID.
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary
)
