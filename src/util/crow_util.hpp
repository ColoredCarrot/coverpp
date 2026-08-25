#pragma once

#include <crow.h>

namespace coverpp::detail
{
crow::response make_file_response(std::filesystem::path const& file, std::string_view content_type = "text/plain");

void run_server(crow::SimpleApp& app, int port, bool open);

}
