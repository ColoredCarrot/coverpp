#include "serve.hpp"
#include "../util/open_browser.hpp"
#include "../util/console_color.hpp"

#include <crow.h>
#include <print>

namespace coverpp
{
static std::optional<std::string> read_file(const std::filesystem::path& file)
{
    std::ifstream ifs(file, std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
    if (!ifs)
    {
        return std::nullopt;
    }

    const auto size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::vector<char> bytes(size);
    ifs.read(bytes.data(), size);

    return std::string{bytes.data(), static_cast<std::size_t>(size)};
}

static crow::response
make_file_response(const std::filesystem::path& file, std::string_view content_type = "text/plain")
{
    if (!std::filesystem::is_regular_file(file))
    {
        return crow::response{404, "text/plain", "404 Not Found"};
    }

    auto content = read_file(file);
    if (!content)
    {
        return crow::response{404, "text/plain", "404 Not Found"};
    }

    crow::response response;
    response.body = *std::move(content);
    response.set_header("Content-Length", std::format("{}", response.body.size()));
    response.set_header("Content-Type", std::string{content_type});
    return response;
}

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
            if (extension.starts_with("."))
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

    auto future = app
        .port(options.port)
        .concurrency(2)
        .loglevel(crow::LogLevel::Warning)
        .run_async();

    app.wait_for_server_start();

	if (options.open)
	{
		detail::open_browser_at_url(std::format("http://localhost:{}", options.port));
	}

    std::println("Serving {}\n@ http://localhost:{}", absolute(options.report_path).u8string(), options.port);
    std::println("{}", styled<ColorBold::green>("Commands:"));
    std::println(" ➜ {} to quit the server", styled<Style::bold>("q + enter"));
    std::println(" ➜ {} to open the default browser", styled<Style::bold>("o + enter"));

    int c;
    while ((c = std::getchar()) != EOF)
    {
        if (c == 'q')
        {
            break;
        }
        if (c == 'o')
        {
            detail::open_browser_at_url(std::format("http://localhost:{}", options.port));
        }
    }

    app.stop();

    future.wait_for(std::chrono::seconds{30});

    return 0;
}
}
