#include "core_api.h"
#include <gtest/gtest.h>

using namespace snes9x;

TEST(CoreSmoke, LoadAndRun) {
    Core c;
    ASSERT_TRUE(c.loadRom("dummy.sfc"));
    auto r = c.run(10);
    EXPECT_TRUE(r.success);
    EXPECT_GE(r.frames_executed, 1);
}

TEST(CoreSmoke, SaveAndLoadState) {
    Core c;
    ASSERT_TRUE(c.loadRom("dummy.sfc"));
    const std::string statefile = "test-state.bin";
    ASSERT_TRUE(c.saveState(statefile));
    ASSERT_TRUE(c.loadState(statefile));
    // remove the statefile to avoid leaving artifacts
    remove(statefile.c_str());
}

TEST(CoreSmoke, DumpMemory) {
    Core c;
    auto buf = c.dumpMemory(0x100, 16);
    ASSERT_EQ(buf.size(), 16);
    EXPECT_EQ(buf[0], static_cast<uint8_t>(0x100 & 0xFF));
}
