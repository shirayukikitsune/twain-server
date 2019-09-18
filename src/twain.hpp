#pragma once

#include <boost/dll.hpp>
#include <boost/function.hpp>

#include "external/twain.h"

namespace dasa::gliese::scanner {

    class Twain {
    public:
        ~Twain();

        void fillIdentity();
        void loadDSM(const char *path);
        void openDSM();
        void closeDSM();

        pTW_IDENTITY getIdentity() {
            return &identity;
        }

        std::list<TW_IDENTITY> listSources();
        TW_IDENTITY getDefaultDataSource();

        boost::function<TW_UINT16(pTW_IDENTITY, pTW_IDENTITY, TW_UINT32, TW_UINT16, TW_UINT16, TW_MEMREF)> entry;

    private:
        TW_STATUS getStatus(TW_UINT16 rc);

        TW_IDENTITY identity{};
        TW_ENTRYPOINT entrypoint{};
        boost::dll::shared_library DSM;
        bool isOpen = false;
    };

}
