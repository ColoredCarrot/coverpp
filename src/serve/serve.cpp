#include "serve.hpp"

#include "../file_util.hpp"
#include "../util/console_color.hpp"
#include "../util/crow_util.hpp"

#include <crow.h>
#include <print>

namespace coverpp
{
using detail::make_file_response;

int serve(const ServeOptions& options)
{
    const std::filesystem::path webapp_path{options.coverpp_install_dir / "webapp"};

    if (!std::filesystem::is_directory(webapp_path))
    {
        throw std::runtime_error{
            std::format("Failed to find webapp at {}\nSpecify an alternative installation directory with {}",
                        webapp_path.u8string(),
                        styled<ColorBold::gray>("--coverpp-install-dir <dir>"))};
    }

    crow::SimpleApp app;

    CROW_ROUTE(app, "/ctx")([&] {
        return crow::json::wvalue({
                                      {"coverppOk", true},
                                  });
    });

    CROW_ROUTE(app, "/ctx/report")([&] {
        return make_file_response(options.report_path, "application/octet-stream");
    });

    CROW_ROUTE(app, "/src/<path>")([](const std::filesystem::path& requested_path) {
        return make_file_response(weakly_canonical(requested_path));
    });

    CROW_ROUTE(app, "/<path>")([&](const std::filesystem::path& requested_path) {
        const auto path = webapp_path / requested_path;

        if (std::filesystem::is_regular_file(path))
        {
            std::string extension = path.extension().string();
            if (extension.starts_with('.'))
            {
                extension.erase(extension.begin());
            }
            return make_file_response(path, crow::response::get_mime_type(extension));
        }
        else
        {
            // Fall back to always serve index.html (client-side routing)
            return make_file_response(webapp_path / "index.html", "text/html; charset=utf-8");
        }
    });

    CROW_ROUTE(app, "/")([&] {
        return make_file_response(webapp_path / "index.html", "text/html; charset=utf-8");
    });

	std::println("Serving {}", canonical(options.report_path).u8string());

	detail::run_server(app, options.port, options.open);

    return 0;
}
}
