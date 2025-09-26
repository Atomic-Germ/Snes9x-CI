#include <cstdio>
#include <cstdlib>
#include "core_api.h"

int main() {
    snes9x::Core c;
    if (!c.loadRom("dummy.sfc")) return 1;
    auto r = c.run(1);
    if (!r.success) return 2;
    auto mem = c.dumpMemory(0x100, 4);
    if (mem.size() != 4) return 3;
    return 0;
}
