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

#define BYTES_PERLINE_ALIGN4(width, bpp) (((((width)*(bpp))+31)/32)*4)

BITMAPINFOHEADER make_info_header(const TW_IMAGEINFO& image_info) {
    BITMAPINFOHEADER info_header{ 0 };

    info_header.biSize = sizeof(BITMAPINFOHEADER);
    info_header.biWidth = image_info.ImageWidth;
    info_header.biHeight = -image_info.ImageLength;
    info_header.biPlanes = 1;
    info_header.biBitCount = image_info.BitsPerPixel;
    info_header.biCompression = 0;
    info_header.biXPelsPerMeter = image_info.XResolution.Whole * 39.37F + 0.5;
    info_header.biYPelsPerMeter = image_info.YResolution.Whole * 39.37F + 0.5;
    switch (image_info.PixelType)
    {
    case TWPT_RGB:
        info_header.biClrUsed = 0;
        break;
    case TWPT_BW:
    case TWPT_GRAY:
    case TWPT_PALETTE:
        info_header.biClrUsed = 1 << image_info.BitsPerPixel;
        break;
    }
    info_header.biClrImportant = info_header.biClrUsed;
    info_header.biSizeImage = BYTES_PERLINE_ALIGN4(info_header.biWidth, info_header.biBitCount) * info_header.biHeight;

    return info_header;
}

bool MemoryTransfer::transferOne(std::ostream& os) {
	TW_IMAGEMEMXFER memXferBuffer;
	TW_UINT16 rc;
    TW_SETUPMEMXFER sourceBufferSize{0};
    TW_IMAGEMEMXFER memXferTemplate{0};

    LOG_S(INFO) << "Getting buffer size";
    memset(&sourceBufferSize, 0, sizeof(TW_SETUPMEMXFER));
    (*twain)(DG_CONTROL, DAT_SETUPMEMXFER, MSG_GET, reinterpret_cast<TW_MEMREF>(&sourceBufferSize));

    auto bufferSize = sourceBufferSize.Preferred;

    LOG_S(INFO) << "Creating memory transfer template (Buffer size of " << bufferSize << " bytes)";

    std::unique_ptr<std::byte[]> buffer = std::make_unique<std::byte[]>(bufferSize);

    memset(&memXferTemplate, 0, sizeof(TW_IMAGEMEMXFER));
    memXferTemplate.Memory.Flags = TWMF_APPOWNS | TWMF_POINTER;
    memXferTemplate.Memory.Length = bufferSize;
    memXferTemplate.Memory.TheMem = buffer.get();

    BITMAPINFOHEADER infoHeader = make_info_header(imageInfo);
    unsigned paletteSize = sizeof(RGBQUAD) * infoHeader.biClrUsed;

    BITMAPFILEHEADER fileHeader{
        0x4d42,
        (uint32_t)(infoHeader.biSizeImage + paletteSize + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER)),
        0,
        0,
        (uint32_t)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + paletteSize)
    };

    os.write(reinterpret_cast<char*>(&fileHeader), sizeof(BITMAPFILEHEADER));
    os.write(reinterpret_cast<char*>(&infoHeader), sizeof(BITMAPINFOHEADER));

    if (infoHeader.biClrUsed) {
        auto palette = std::make_unique<RGBQUAD[]>(infoHeader.biClrUsed);
        memset(palette.get(), 0, sizeof(RGBQUAD) * infoHeader.biClrUsed);

        if (imageInfo.PixelType == TWPT_PALETTE) {
            TW_PALETTE8 twainPalette{ 0 };
            rc = (*twain)(DG_IMAGE, DAT_PALETTE8, MSG_GET, reinterpret_cast<TW_MEMREF>(&twainPalette));
            if (rc != TWRC_SUCCESS) {
                LOG_S(ERROR) << "Error while transfering palette from DS";
                buffer = nullptr;
                return false;
            }

            if (twainPalette.PaletteType == TWPA_RGB || twainPalette.PaletteType == TWPA_GRAY) {
                if (infoHeader.biClrUsed != twainPalette.NumColors)
                {
                    infoHeader.biClrUsed = twainPalette.NumColors;
                    palette = std::make_unique<RGBQUAD[]>(infoHeader.biClrUsed);
                    memset(palette.get(), 0, sizeof(RGBQUAD) * infoHeader.biClrUsed);
                }

                for (int nIndex = 0; nIndex < twainPalette.NumColors; nIndex++)
                {
                    palette[nIndex].rgbRed = twainPalette.Colors[nIndex].Channel1;
                    palette[nIndex].rgbGreen = twainPalette.Colors[nIndex].Channel2;
                    palette[nIndex].rgbBlue = twainPalette.Colors[nIndex].Channel3;
                }
            }
        }
        else {
            int nIncr = 0xFF / (infoHeader.biClrUsed - 1);
            int nVal = 0;
            for (int nIndex = 0; nIndex < infoHeader.biClrUsed; nIndex++) {
                //create the palette
                nVal = nIndex * nIncr;
                palette[nIndex].rgbRed = static_cast<uint8_t>(nVal);
                palette[nIndex].rgbGreen = static_cast<uint8_t>(nVal);
                palette[nIndex].rgbBlue = static_cast<uint8_t>(nVal);
            }
        }
        os.write(reinterpret_cast<char*>(palette.get()), sizeof(RGBQUAD) * infoHeader.biClrUsed);
    }

    os.flush();

	LOG_S(INFO) << "Starting transfer";
	while (true) {
		memcpy(&memXferBuffer, &memXferTemplate, sizeof(TW_IMAGEMEMXFER));
		memset(memXferBuffer.Memory.TheMem, 0, memXferBuffer.Memory.Length);

		rc = (*twain)(DG_IMAGE, DAT_IMAGEMEMXFER, MSG_GET, reinterpret_cast<TW_MEMREF>(&memXferBuffer));

		if (rc == TWRC_CANCEL) {
			LOG_S(WARNING) << "Cancelled transfer while trying to get data";
			break;
		}
		if (rc == TWRC_FAILURE) {
			LOG_S(ERROR) << "Error while transfering data from DS";
			break;
		}

		twain->setState(7);

        os.write(reinterpret_cast<char*>(memXferBuffer.Memory.TheMem), memXferBuffer.BytesWritten);
        os.flush();

		if (rc == TWRC_XFERDONE) {
			break;
		}
	}

	buffer = nullptr;

    return rc == TWRC_XFERDONE;
}
