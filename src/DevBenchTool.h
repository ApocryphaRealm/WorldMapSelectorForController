#pragma once

namespace WMS::DevBenchTool
{
    // Registers "wmsfc.control" when DevBench/TestBench is loaded. lastAttempt = true on the final retry (kDataLoaded).
    void Init(bool lastAttempt);
}
