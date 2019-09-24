#include "router.hpp"

using dasa::gliese::scanner::http::Router;
using namespace boost::beast::http;

response<string_body> Router::handle_request(request<string_body>&& request) {
	auto handler = handlers.find(request.target());
	if (handler == handlers.end()) {
		return std::move(makeNotFoundResponse(request));
	}

	return std::move((*handler->second)(std::move(request)));
}

response<string_body> Router::makeNotFoundResponse(const request<string_body>& request) {
	response<string_body> res{ status::not_found, request.version() };
	res.set(field::server, BOOST_BEAST_VERSION_STRING);
	res.set(field::content_type, "text/plain");
	res.keep_alive(request.keep_alive());
	res.body() = "not found";
	res.prepare_payload();
	return res;
}
