#include "devices.hpp"
#include "handlers.hpp"
#include "../../application.hpp"

extern dasa::gliese::scanner::Application *application;

using dasa::gliese::scanner::http::handler::DevicesHandler;
using nlohmann::json;
namespace bh = boost::beast::http;

bh::response<bh::dynamic_body> DevicesHandler::operator()(bh::request<bh::string_body>&& request) {
	json response;
    auto devices = application->getTwain().listSources();
    auto defaultDevice = application->getTwain().getDefaultDataSource();
    size_t i = 0;
    for (auto & device : devices) {
        auto deviceJson = deviceToJson(device);
        if (defaultDevice.Id == device.Id) {
            deviceJson["default"] = true;
        }
        response[i++] = deviceJson;
    }
        
    bh::response<bh::dynamic_body> res{ bh::status::ok, request.version() };
	res.set(bh::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(bh::field::content_type, "application/json");
	res.keep_alive(request.keep_alive());
	boost::beast::ostream(res.body()) << response;
	res.prepare_payload();
	return res;
}
