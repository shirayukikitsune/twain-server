/*
    twain-server: This project exposes the TWAIN DSM as a web API
    Copyright (C) 2019 "Diagnósticos da América S.A. <bruno.f@dasa.com.br>"

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <boost/asio.hpp>
#include <memory>
#include "http/listener.hpp"
#include "twain.hpp"

namespace dasa::gliese::scanner
{
    /**
     * Base class for all application specific code
     */
    class Application {
    public:
        /**
         * Initializes the application
         *
         * @param httpListener An instance of the HTTP Listener
         */
        virtual void initialize(std::shared_ptr<http::Listener> httpListener);

        /**
         * Runs the application
         *
         * This is where the application starts listening to incoming connections and handle them
         */
        virtual void run() = 0;

        /**
         * Stops the running application
         *
         * Here the listener should be closed and all related resources, freed
         */
        virtual void stop() = 0;

        /**
         * Fetches the reference to the TWAIN instance
         *
         * @return The twain instance
         */
        virtual Twain& getTwain() = 0;

        /**
         * Fetches the I/O context for all I/O operations except for TWAIN
         * @return The I/O context
         */
        boost::asio::io_context& getIoContext() { return ioc; }

        /**
         * Fetches the I/O context for TWAIN operations
         */
        boost::asio::io_context& getTwainIoContext() { return twain_ioc; }

        /**
         * Returns a handle to the main window of this application
         */
        virtual TW_HANDLE getParentWindow() { return nullptr; }

        /**
         * Returns a reference to the handle of the main window
         * @return
         */
        virtual TW_MEMREF getParentWindowRef() { return nullptr; }

    protected:
        std::shared_ptr<http::Listener> listener;
        boost::asio::io_context ioc;
        boost::asio::io_context twain_ioc;
    };
}
