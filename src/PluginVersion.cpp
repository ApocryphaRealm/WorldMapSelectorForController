#include "REL/Version.h"
#include "SKSE/SKSE.h"

SKSEPluginInfo(
    .Version = REL::Version{ WMS_VERSION_MAJOR, WMS_VERSION_MINOR, WMS_VERSION_PATCH, WMS_VERSION_TWEAK },
    .Name = "WorldMapSelectorForController"sv,
    .Author = "ApocryphaRealm"sv,
    .SupportEmail = ""sv,
    .StructCompatibility = SKSE::StructCompatibility::Independent,
    .RuntimeCompatibility = {
        REL::Version{ 1, 5, 97, 0 },
        REL::Version{ 1, 6, 318, 0 },
        REL::Version{ 1, 6, 323, 0 },
        REL::Version{ 1, 6, 342, 0 },
        REL::Version{ 1, 6, 353, 0 },
        REL::Version{ 1, 6, 629, 0 },
        REL::Version{ 1, 6, 640, 0 },
        REL::Version{ 1, 6, 1130, 0 },
        REL::Version{ 1, 6, 1170, 0 },
        REL::Version{ 1, 7, 99, 0 },
        REL::Version{ 1, 7, 104, 0 }
    }
)
