#include "status.hpp"
#include "handlers.hpp"
#include "../../application.hpp"

extern dasa::gliese::scanner::Application *application;

using dasa::gliese::scanner::http::handler::StatusHandler;
namespace bh = boost::beast::http;

bh::response<bh::dynamic_body> StatusHandler::operator()(bh::request<bh::string_body>&& request) {
    nlohmann::json response;
    response["status"] = "UP";
	response["details"] = {
		{"twain", application->getTwain().getState()}
	};

    bh::response<bh::dynamic_body> res{ bh::status::ok, request.version() };
	res.set(bh::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(bh::field::content_type, "application/json");
	res.keep_alive(request.keep_alive());
	boost::beast::ostream(res.body()) << response;
	res.prepare_payload();
	return res;
}
