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

#include <fstream>
#include <memory>
#include <utility>
#include "../transfer.hpp"

namespace dasa::gliese::scanner::twain {

    class conversion_transfer : public Transfer {
    public:
        conversion_transfer(dasa::gliese::scanner::Twain *twain, std::string outputMime)
        : Transfer(twain, std::move(outputMime)) {}

        void set_transfer(std::shared_ptr<Transfer> transfer) {
            original_transfer = std::move(transfer);
        }

        TW_IMAGEINFO prepare() final {
            return original_transfer->prepare();
        }

        bool transferOne(std::ostream& outputStream) final;

        std::string getTransferMIME() final {
            return outputMime == "*/*" ? getDefaultMIME() : outputMime;
        }
        std::string getDefaultMIME() final {
            return "image/jpeg";
        }

    private:
        std::shared_ptr<Transfer> original_transfer;

        static bool convert_image(const std::string& from, const std::string& to);
        static std::string create_temp_file();
        static std::string get_extension_for_mime(const std::string& mime);

        static void write_file_to_stream(std::ostream &os, const std::string &output_file_name);
    };

}
