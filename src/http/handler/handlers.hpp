#pragma once

#include "handler.hpp"
#include "../listener.hpp"

void create_handlers();
void install_handlers(const std::shared_ptr<dasa::gliese::scanner::http::Listener> &listener);
