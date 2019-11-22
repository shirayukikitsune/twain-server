#include "cors.hpp"

namespace dasa::gliese::scanner::http::handler {
    namespace bh = boost::beast::http;

    void add_cors(const bh::request<bh::string_body>& request, bh::response<bh::dynamic_body>& response) {
        auto origin = request[bh::field::origin];
        if (!origin.empty()) {
            response.set(bh::field::access_control_allow_origin, origin);
        }
    }
}
