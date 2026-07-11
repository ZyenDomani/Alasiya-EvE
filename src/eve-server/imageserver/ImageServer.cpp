/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    Copyright 2016 - 2026 Alasiya-EvE by Allan
    For the latest implementation status visit http://eve.alasiya.net/?p=op_status
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        caytchen
    Updates:    Allan
*/


#include "imageserver/ImageServer.h"
#include "imageserver/ImageServerListener.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// SAFE GLOBAL SCOPE: This prevents the "non-static member function" compiler error
static void WriteBufferCallback(void *context, void *data, int len) {
    std::vector<char> *buffer = static_cast<std::vector<char>*>(context);
    const char *charData = static_cast<const char*>(data);
    buffer->insert(buffer->end(), charData, charData + len);
}

//const char *const ImageServer::FallbackURL = "http://eve-images.alasiya.net/";
const char *const ImageServer::FallbackURL = "http://images.evetech.net/";

const char *const ImageServer::Categories[] = {
    "Agent",
    "Alliance",
    "Corporation",
    "Character",
    "InventoryType",
    "Render" };

const uint32 ImageServer::CategoryCount = 6;

void ImageServer::Init() {
    std::stringstream urlBuilder;
    urlBuilder << "http://" << sConfig.net.imageServer << ":" << sConfig.net.imageServerPort << "/";
    _url = urlBuilder.str();

    _basePath = sConfig.files.imageDir;
    if (_basePath[_basePath.size() - 1] != '/')
        _basePath += "/";

    sLog.Cyan("      ImageServer", "Image Server URL: %s", _url.c_str());
    sLog.Cyan("      ImageServer", "Image Server path: %s", _basePath.c_str());

    if (CreateDirectory( _basePath.c_str(), NULL ) == 0) {
        for (int i = 0; i < CategoryCount; i++) {
            std::string subdir = _basePath;
            subdir.append(Categories[i]);
            CreateDirectory( subdir.c_str(), NULL );
        }
    }

    sLog.Blue("      ImageServer", "Image Server Initalized.");
}

void ImageServer::ReportNewImage(uint32 accountID, std::shared_ptr<std::vector<char> > imageData)
{
    sLog.Warning("      ImageServer"," ReportNewImage() called.");
    Lock lock(_limboLock);

    if (_limboImages.find(accountID) != _limboImages.end()) {
        _limboImages.insert(std::pair<uint32,std::shared_ptr<std::vector<char> > >(accountID, imageData));
    } else {
        _limboImages[accountID] = imageData;
    }
}

void ImageServer::ReportNewCharacter(uint32 creatorAccountID, uint32 characterID)
{
    sLog.Warning("      ImageServer"," ReportNewCharacter() called.");
    Lock lock(_limboLock);

    // check if we received an image from this account previously
    if (_limboImages.find(creatorAccountID) == _limboImages.end()) {
        sLog.Error("      ImageServer"," Image not received for characterID %u.", characterID);
        /** @todo  need to get client here, and send msg about emailing char pic and name to charPics@eve.alasiya.net for manual insertion */
        return;
    }

    // we have, so save it
    //std::ofstream stream;
    std::string dirName = "Character";
    std::string path(GetFilePath(dirName, characterID, 512));
    FILE * fp = fopen(path.c_str(), "wb");

    //stream.open(path, std::ios::binary | std::ios::trunc | std::ios::out);
    std::shared_ptr<std::vector<char> > data = _limboImages[creatorAccountID];

    fwrite(&((*data)[0]), 1, data->size(), fp);
    fclose(fp);

    // and delete it from our limbo map
    _limboImages.erase(creatorAccountID);

    sLog.Green("      ImageServer", "Received image from accountID %u and saved as %s", creatorAccountID, path.c_str());
}

std::shared_ptr<std::vector<char>> ImageServer::GetImage(std::string& category, uint32 id, uint32 size) {
    //sLog.Cyan("      ImageServer", " GetImage() called. Cat: %s, id: %u, size:%u", category.c_str(), id, size);

    if (!ValidateCategory(category) || !ValidateSize(category, size))
        return std::shared_ptr<std::vector<char>>();

    // 1. Attempt to scrape the exact resolution file requested by the client from disk
    std::string path(GetFilePath(category, id, size));
    FILE* fp = fopen(path.c_str(), "rb");

    // ==========================================
    // 🛡️ THE ARCHITECTURAL FALLBACK GATE
    // ==========================================
    if (fp == NULL) {
        bool isAgent = (id >= 3000000 && id < 4000000);
        uint32 masterSize = isAgent ? 128 : 512; // Agents scale from 128px, players from 512px

        if (size == masterSize)
            return std::shared_ptr<std::vector<char>>(); // Prevent circular allocations

        // Fetch the file path for your high-res master asset cleanly through your refactored method
        std::string masterPath(GetFilePath(category, id, masterSize));
        FILE* masterFp = fopen(masterPath.c_str(), "rb");

        // Secondary Fallback: If your download script is still running and the 128px version
        // doesn't exist on disk yet, drop back to using your legacy 64px backup file as the source
        if (masterFp == NULL && isAgent) {
            masterSize = 64;
            masterPath = GetFilePath(category, id, masterSize);
            masterFp = fopen(masterPath.c_str(), "rb");
        }

        if (masterFp == NULL) {
            sLog.Error("      ImageServer", " Asset Missing: ID %u cannot be resolved at size %u or master size %u", id, size, masterSize);
            return std::shared_ptr<std::vector<char>>();
        }
        fclose(masterFp);

        //sLog.Warning("      ImageServer", " Size %u missing on disk. Scaling on-the-fly via %upx master file.", size, masterSize);

        // 1. Decode the master image straight into a raw RGB pixel stream
        int width, height, channels;
        unsigned char* masterPixels = stbi_load(masterPath.c_str(), &width, &height, &channels, 0);
        if (!masterPixels) return std::shared_ptr<std::vector<char>>();

        // 2. Allocate an uninitialized local buffer for the downscaled target pixels
        std::vector<unsigned char> scaledPixels(size * size * channels);

        // =======================================================
        // 🚀 THE NEW v2.x STB RESIZE API CALL
        // =======================================================
        // Parameters: input_pixels, input_w, input_h, input_stride_in_bytes (0 calculates auto),
        //             output_pixels, output_w, output_h, output_stride_in_bytes (0 calculates auto),
        //             pixel_layout (cast channels directly to specify RGB/RGBA/etc.)
        stbir_resize_uint8_linear(masterPixels, width, height, 0,
                                  scaledPixels.data(), size, size, 0,
                                  (stbir_pixel_layout)channels);

        // Clean up the master decoder heap allocations immediately
        stbi_image_free(masterPixels);

        // 3. Re-encode the raw pixels back into a valid compressed JPEG
        std::shared_ptr<std::vector<char>> ret = std::make_shared<std::vector<char>>();

        // This will compile flawlessly now because WriteBufferCallback is a clean static global pointer!
        stbi_write_jpg_to_func(WriteBufferCallback, ret.get(), size, size, channels, scaledPixels.data(), 90);

        return ret;
    }

    // ==========================================
    // Your Existing Stable, High-Speed File Reader
    // ==========================================
    fseek(fp, 0, SEEK_END);
    size_t length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // C++11 std::make_shared is cleaner and faster than raw 'new' tracking allocation syntax
    std::shared_ptr<std::vector<char>> ret = std::make_shared<std::vector<char>>();
    ret->resize(length);

    size_t bytesRead = fread(&((*ret)[0]), 1, length, fp);
    fclose(fp);

    return ret;
}

std::string ImageServer::GetFilePath(const std::string& category, uint32 id, uint32 size) {
    std::stringstream builder;

    // 1. Core Fix: Check if the ID belongs to an NPC Agent character block (3,000,000 to 3,999,999)
    bool isAgent = (id >= 3000000 && id < 4000000);

    if (isAgent) {
        // 2. AGENT DIRECTORY ROUTING
        // Normal agent portraits are exclusively .jpg files
        std::string extension = "jpg";

        if (size == 64) {
            // Point straight to your legacy agent backup directory
            builder << _basePath << "Agent64/" << id << "_64." << extension;
        } else {
            // Point straight to your new high-res master asset directory (e.g., size 128, 50, etc.)
            builder << _basePath << "Agent/" << id << "_" << size << "." << extension;
        }
    } else {
        // 3. STANDARD CATEGORY ROUTING (Players, Renders, Alliance, Corp, Inventory)
        // Renders, Corporations, Alliances, and Inventory items use transparent .png files
        // Real player "Character" profiles use standard compressed .jpg files
        std::string extension = (category == "Character") ? "jpg" : "png";

        builder << _basePath << category << "/" << id << "_" << size << "." << extension;
    }

    return builder.str();
}

bool ImageServer::ValidateSize(std::string& category, uint32 size)
{
    if (size < 16)
        return false;
    if (category == "InventoryType")
        return (size <= 64);

    if ((category == "Alliance") or (category == "Corporation") or (category == "Agent"))
        return (size <= 256);

    // Render and Character
    return (size <= 512);
}

bool ImageServer::ValidateCategory(std::string& category)
{
    for (int i = 0; i < 6; ++i)
        if (category == Categories[i])
            return true;
    return false;
}

std::string& ImageServer::url()
{
    return _url;
}

void ImageServer::Run()
{
    _ioThread = std::shared_ptr<boost::asio::detail::thread>(new boost::asio::detail::thread(std::bind(&ImageServer::RunInternal, this)));
}

void ImageServer::Stop()
{
    _io->stop();
    _ioThread->join();
}

void ImageServer::RunInternal()
{
    _io = std::shared_ptr<boost::asio::io_service>(new boost::asio::io_service());
    _listener = std::shared_ptr<ImageServerListener>(new ImageServerListener(*_io));
    _io->run();
}

ImageServer::Lock::Lock(boost::asio::detail::mutex& mutex)
: _mutex(mutex)
{
    _mutex.lock();
}

ImageServer::Lock::~Lock()
{
    _mutex.unlock();
}
