#include <iostream>
#include <loguru.hpp>

#include "application.hpp"

extern dasa::gliese::scanner::Application *application;

web::json::value deviceToJson(TW_IDENTITY device) {
    auto deviceJson = web::json::value::object();
    deviceJson[U("id")] = web::json::value((uint32_t)device.Id);
    deviceJson[U("productName")] = web::json::value(utility::conversions::to_string_t(device.ProductName));
    deviceJson[U("manufacturer")] = web::json::value(utility::conversions::to_string_t(device.Manufacturer));
    deviceJson[U("productFamily")] = web::json::value(utility::conversions::to_string_t(device.ProductFamily));
    return deviceJson;
}

int main(int argc, char **argv) {
    loguru::init(argc, argv, "-v");

    auto listener = std::make_shared<dasa::gliese::scanner::http::Listener>();

    listener->get(U("/status"), [](const web::http::http_request& request) {
        auto response = web::json::value::object();
        response[U("status")] = web::json::value(U("UP"));
        auto details = web::json::value::object();
        details[U("twain")] = web::json::value(application->getTwain().isReady() ? U("UP") : U("DOWN"));
        response[U("details")] = web::json::value(details);
        request.reply(web::http::status_codes::OK, response);
    });
    listener->get(U("/devices"), [](const web::http::http_request& request) {
        auto response = web::json::value::array();
        auto devices = application->getTwain().listSources();
        auto defaultDevice = application->getTwain().getDefaultDataSource();
        size_t i = 0;
        for (auto & device : devices) {
            auto deviceJson = deviceToJson(device);
            if (defaultDevice.Id == device.Id) {
                deviceJson[U("default")] = web::json::value(true);
            }
            response[i++] = deviceJson;
        }
        request.reply(web::http::status_codes::OK, response);
    });

    listener->initialize(U("http://127.0.0.1:43456"));

    LOG_F(INFO, "Opening HTTP listener on 127.0.0.1:43456");

    application->initialize(listener);

    LOG_F(INFO, "Application initialized");

    LOG_S(INFO) << "Connecting to TWAIN";
    application->getTwain().fillIdentity();
    application->getTwain().openDSM();

    LOG_S(INFO) << "Listing TWAIN devices";
    auto devices = application->getTwain().listSources();
    for (auto & device : devices) {
        LOG_S(INFO) << device;
    }

    auto device = application->getTwain().getDefaultDataSource();
    LOG_S(INFO) << "Default device: " << device;

    application->run();

    LOG_F(INFO, "Closing TWAIN DSM");
    application->getTwain().closeDSM();
}

std::ostream& operator<<(std::ostream& os, const TW_IDENTITY& identity) {
    os << "Device " << identity.Id;
    if (identity.Id != 0) {
        os << ": " << identity.Manufacturer << ", " << identity.ProductName << ", " << identity.ProductFamily;
    } else {
        os << " has no data";
    }
    return os;
}
