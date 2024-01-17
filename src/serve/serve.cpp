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

int serve(const ServeOptions& options)
{
    const std::filesystem::path webapp_path{""};

    crow::SimpleApp app;

    CROW_ROUTE(app, "/src/<path>")([&](const std::filesystem::path& requested_path) {
        const auto path = weakly_canonical(requested_path);

        if (!std::filesystem::is_regular_file(path))
        {
            return crow::response{404, "text/plain", "404 Not Found"};
        }

        auto content = read_file(path);
        if (!content)
        {
            return crow::response{404, "text/plain", "404 Not Found"};
        }

        crow::response response;
        response.body = *std::move(content);
        response.set_header("Content-Length", std::format("{}", response.body.size()));
        response.set_header("Content-Type", "text/plain");
        return response;
    });

    auto future = app
        .port(options.port)
        .concurrency(2)
        .loglevel(crow::LogLevel::Warning)
        .run_async();

    app.wait_for_server_start();

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
