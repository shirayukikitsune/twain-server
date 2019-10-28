#include <bandit/bandit.h>
#include <loguru.hpp>
#include <csignal>

#include "twain/dsm.hpp"

#ifndef TWH_CMP_GNU
#define TWAIN_LIBRARY "TWAINDSM.dll"
#else
#define TWAIN_LIBRARY "/usr/local/lib/libtwaindsm.so"
#endif

using namespace snowhouse;
using namespace bandit;
using namespace dasa::gliese::scanner::twain;

TW_IDENTITY appIdentity{
    0,
    { 1, 0, TWLG_ENGLISH, TWCY_BRAZIL, "1.0.0" },
    2, 4,
    DF_APP2 | DG_CONTROL | DG_IMAGE,
    "Diagnosticos da America SA",
    "TWAIN server",
    "DSM Tests"
};



go_bandit([] {
    describe("DSM state", [] {
        DSM dsm;

        it("should start unloaded", [&] {
            AssertThat(dsm.state(), Equals(DSM::State::Unloaded));
        });
    });

    describe("DSM library loading", [] {
        DSM dsm;

        before_each([&] {
            dsm.path(TWAIN_LIBRARY);
        });

        after_each([&] {
            dsm.unload();
        });

        it("should have a valid state", [&] {
            auto success = dsm.load();

            AssertThat(success, Equals(true));
            AssertThat(dsm.state(), Equals(DSM::State::Disconnected));
        });

        it("should not change state if library loading failed", [&] {
            dsm.path("some invalid path");
            auto success = dsm.load();

            AssertThat(success, Equals(false));
            AssertThat(dsm.state(), Equals(DSM::State::Unloaded));
        });
    });

    describe("DSM library unloading", [] {
        DSM dsm;

        before_each([&] {
            dsm.path(TWAIN_LIBRARY);
            auto ignored = dsm.load();
        });

        it("should have a valid state", [&] {
            dsm.unload();

            AssertThat(dsm.state(), Equals(DSM::State::Unloaded));
        });
    });

    describe("DSM connection opening", [] {
        DSM dsm;

        before_each([&] {
            dsm.path(TWAIN_LIBRARY);
            auto ignored = dsm.load();
        });

        it("should open successfully a connection", [&] {
            auto success = dsm.open(&appIdentity, nullptr);

            AssertThat(success, Equals(true));
            AssertThat(dsm.state(), Equals(DSM::State::Ready));
        });
    });

    describe("DSM_entry", [] {
        DSM dsm;

        before_each([&] {
            dsm.path(TWAIN_LIBRARY);
            auto ignored = dsm.load();
        });

        it("should fail when DSM is unloaded", [&] {
            dsm.unload();

            auto result = dsm(&appIdentity, nullptr, DG_CONTROL, DAT_STATUS, MSG_GET, nullptr);
            AssertThat(result, Equals(TWRC_FAILURE));
        });
    });
});

int main(int argc, char** argv) {
    // Disable logging
    loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
    bandit::run(argc, argv);
}
