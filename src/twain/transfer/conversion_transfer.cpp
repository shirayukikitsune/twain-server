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

#include "conversion_transfer.hpp"

#include <filesystem>
#include <loguru.hpp>
#include <Magick++.h>

using namespace dasa::gliese::scanner::twain;
using namespace std::literals;
namespace fs = std::filesystem;

bool conversion_transfer::transferOne(std::ostream &outputStream) {
    // Check if conversion is needed
    if (outputMime == original_transfer->getTransferMIME()) {
        return original_transfer->transferOne(outputStream);
    }

    LOG_SCOPE_FUNCTION(INFO);
    auto file_name = create_temp_file();

    LOG_S(INFO) << "Creating temporary file " << file_name;
    std::ofstream temp_file_stream(file_name, std::ios::binary | std::ios::trunc);
    auto success = original_transfer->transferOne(temp_file_stream);
    temp_file_stream.close();

    if (success) {
        auto output_file_name = file_name + get_extension_for_mime(outputMime);
        success = convert_image(file_name, output_file_name);
        if (success) {
            write_file_to_stream(outputStream, output_file_name);

            fs::remove(output_file_name);
        }
    }

    fs::remove(file_name);

    return success;
}

bool conversion_transfer::convert_image(const std::string &from, const std::string &to) {
    try {
        Magick::Image image;
        image.read(from);
        image.write(to);
        return true;
    } catch (Magick::Exception& e) {
        LOG_S(ERROR) << "Failed to convert image: " << e.what();
    }

    return false;
}

std::string conversion_transfer::create_temp_file() {
    std::error_code ec;
    auto temp_path = fs::temp_directory_path(ec);
    if (ec) {
        return ""s;
    }
    auto file_name = fs::absolute(temp_path.generic_string() + fs::path::preferred_separator + "twain-server-transfer");
    return file_name;
}

std::string conversion_transfer::get_extension_for_mime(const std::string &mime) {
    if (mime == "image/jpeg") {
        return ".jpeg"s;
    }
    if (mime == "image/bmp") {
        return ".bmp"s;
    }
    if (mime == "image/tiff") {
        return ".tiff"s;
    }
    return ""s;
}

void conversion_transfer::write_file_to_stream(std::ostream &os, const std::string &output_file_name) {
    std::ifstream converted(output_file_name, std::ios::binary);
    os << converted.rdbuf();
    os.flush();
    converted.close();
}
