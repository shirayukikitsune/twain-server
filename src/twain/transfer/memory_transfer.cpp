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

#include <loguru.hpp>
#include "memory_transfer.hpp"

using namespace dasa::gliese::scanner::twain;

TW_IMAGEINFO MemoryTransfer::prepare() {

	LOG_S(INFO) << "Getting image info";
	memset(&imageInfo, 0, sizeof(TW_IMAGEINFO));
	(*twain)(DG_IMAGE, DAT_IMAGEINFO, MSG_GET, reinterpret_cast<TW_MEMREF>(&imageInfo));

	LOG_S(INFO) << "Getting buffer size";
	memset(&sourceBufferSize, 0, sizeof(TW_SETUPMEMXFER));
    (*twain)(DG_CONTROL, DAT_SETUPMEMXFER, MSG_GET, reinterpret_cast<TW_MEMREF>(&sourceBufferSize));

    TW_CAPABILITY byte_order{ICAP_BITORDER, 0, nullptr};
    twain->getCapability(byte_order, MSG_GETCURRENT);
    little_endian = static_cast<TW_INT16>(((pTW_ONEVALUE)byte_order.hContainer)->Item) == TWBO_MSBFIRST;

	auto bufferSize = sourceBufferSize.Preferred;

	LOG_S(INFO) << "Creating memory transfer template (Buffer size of " << bufferSize << " bytes)";
	memset(&memXferTemplate, 0, sizeof(TW_IMAGEMEMXFER));
	memXferTemplate.Compression = TWON_DONTCARE16;
	memXferTemplate.BytesPerRow = TWON_DONTCARE32;
	memXferTemplate.Columns = TWON_DONTCARE32;
	memXferTemplate.Rows = TWON_DONTCARE32;
	memXferTemplate.XOffset = TWON_DONTCARE32;
	memXferTemplate.YOffset = TWON_DONTCARE32;
	memXferTemplate.BytesWritten = TWON_DONTCARE32;
	memXferTemplate.Memory.Flags = TWMF_APPOWNS | TWMF_HANDLE;
	memXferTemplate.Memory.Length = bufferSize;

	return imageInfo;
}

#ifndef _WINDOWS
typedef struct tagBITMAPINFOHEADER {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} __attribute__((packed)) BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagRGBQUAD {
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;
} __attribute__((packed)) RGBQUAD;

typedef struct tagBITMAPFILEHEADER {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} __attribute__((packed)) BITMAPFILEHEADER;
#endif

#define BYTES_PERLINE_ALIGN4(width, bpp) (((((int)(width)*(bpp))+31)/32)*4)

BITMAPINFOHEADER make_info_header(const TW_IMAGEINFO& image_info) {
    BITMAPINFOHEADER info_header{ 0 };

    auto bpp = image_info.BitsPerPixel;
    auto color_count = bpp == 1 ? 2 : bpp == 8 ? 256 : 0;

    if (bpp == 24) {
        bpp = 32;
    }

    info_header.biSize = sizeof(BITMAPINFOHEADER);
    info_header.biWidth = image_info.ImageWidth;
    info_header.biHeight = image_info.ImageLength;
    info_header.biPlanes = 1;
    info_header.biBitCount = bpp;
    info_header.biCompression = 0;
    info_header.biXPelsPerMeter = floor(image_info.XResolution.Whole * 39.3700787 + 0.5);
    info_header.biYPelsPerMeter = floor(image_info.YResolution.Whole * 39.3700787 + 0.5);
    info_header.biClrUsed = color_count;
    info_header.biClrImportant = color_count;
    info_header.biSizeImage = 0;

    return info_header;
}

void write_palette_color(std::ostream& os, uint8_t red, uint8_t green, uint8_t blue) {
    RGBQUAD color{blue,green,red,0};

    os.write(reinterpret_cast<char*>(&color), sizeof(RGBQUAD));
}

void write_palette(std::ostream& os, uint16_t bpp) {
    if (bpp == 1) {
        write_palette_color(os, 0, 0, 0);
        write_palette_color(os, 0xFF, 0xFF, 0xFF);
    }
    else if (bpp == 8) {
        for (int gray = 0; gray <= 255; gray++) {
            write_palette_color(os, gray, gray, gray);
        }
    }
}

void write_file_header(std::ostream& os, BITMAPINFOHEADER& info_header) {
    unsigned palette_size = sizeof(RGBQUAD) * info_header.biClrUsed;
    auto size = BYTES_PERLINE_ALIGN4(info_header.biWidth, info_header.biBitCount) * info_header.biHeight;
    BITMAPFILEHEADER file_header{
        0x4d42,
        (uint32_t)(size + palette_size + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER)),
        0,
        0,
        (uint32_t)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + palette_size)
    };

    os.write(reinterpret_cast<char*>(&file_header), sizeof(BITMAPFILEHEADER));
}

bool MemoryTransfer::transferOne(std::ostream& os) {
	buffer = twain->dsm().alloc(memXferTemplate.Memory.Length);
	memXferTemplate.Memory.TheMem = twain->dsm().lock(buffer);

	auto bpp = imageInfo.BitsPerPixel;
    BITMAPINFOHEADER infoHeader = make_info_header(imageInfo);

    write_file_header(os, infoHeader);
    os.write(reinterpret_cast<char*>(&infoHeader), sizeof(BITMAPINFOHEADER));

    write_palette(os, bpp);

	LOG_S(INFO) << "Starting transfer";
    twain->setState(7);

    std::error_code ec;
	while (transfer_strip(os, ec)) {	}

	twain->dsm().unlock(buffer);
	twain->dsm().free(buffer);
	buffer = nullptr;

    return ec == twain::error_code::transfer_done;
}

void write_little_endian_row(std::ostream& os, char* row_buffer, uint32_t row_size, int16_t bpp) {
    for (auto j = row_size; j > 0; --j) {
        for (auto i = bpp - 1; i >= 0; --i) {
            os.put(row_buffer[(j - 1) * bpp + i]);
        }
        if (bpp == 3) {
            os.put(0);
        }
    }
    os.flush();
}

void write_little_endian(std::ostream& os, char* buffer, TW_IMAGEMEMXFER& transfer, int16_t bpp) {
#if 0
    auto row_bytes_aligned = transfer.BytesPerRow;
    row_bytes_aligned += row_bytes_aligned % 4;
#endif
    for (auto i = 0U; i < transfer.Rows; ++i) {
        write_little_endian_row(os, buffer + i * transfer.BytesPerRow, transfer.Columns, bpp);
    }
}

void write_big_endian(std::ostream& os, char* buffer, TW_IMAGEMEMXFER& transfer, int16_t bpp) {
    auto row_bytes_aligned = transfer.BytesPerRow;
    row_bytes_aligned += row_bytes_aligned % 4;
    for (auto i = 0U; i < transfer.Rows; ++i) {
        for (auto j = 0; j < transfer.Columns; ++j) {
            os.write(buffer + i * row_bytes_aligned + j * bpp, bpp);
            if (bpp == 3) {
                os.put(0);
            }
        }
    }
}

bool MemoryTransfer::transfer_strip(std::ostream& os, std::error_code& ec) {
    TW_IMAGEMEMXFER transfer_buffer;

    memcpy(&transfer_buffer, &memXferTemplate, sizeof(TW_IMAGEMEMXFER));
    memset(transfer_buffer.Memory.TheMem, 0, transfer_buffer.Memory.Length);

    auto rc = (*twain)(DG_IMAGE, DAT_IMAGEMEMXFER, MSG_GET, reinterpret_cast<TW_MEMREF>(&transfer_buffer));

    if (rc == TWRC_CANCEL) {
        LOG_S(WARNING) << "Cancelled transfer while trying to get data";
        ec = twain::make_error_code(twain::error_code::cancelled);
        return false;
    }
    if (rc == TWRC_FAILURE) {
        LOG_S(ERROR) << "Error while transfering data from DS";
        ec = twain::make_error_code(twain::error_code::generic_failure);
        return false;
    }

    auto write_fn = little_endian ? write_little_endian : write_big_endian;
    auto ptr = (char*)transfer_buffer.Memory.TheMem;
    write_fn(os, ptr, transfer_buffer, imageInfo.SamplesPerPixel);
    os.flush();

    if (rc == TWRC_XFERDONE) {
        ec = twain::make_error_code(twain::error_code::transfer_done);
    }
    return rc != TWRC_XFERDONE;
}
