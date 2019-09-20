#pragma once

#include <cpprest/http_msg.h>
#include "../listener.hpp"

void create_handlers();
void install_handlers(const std::shared_ptr<dasa::gliese::scanner::http::Listener> &listener);
