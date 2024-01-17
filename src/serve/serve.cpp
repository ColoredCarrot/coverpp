#include "serve.hpp"
#include "../util/open_browser.hpp"
#include "../util/console_color.hpp"

#include <crow.h>
#include <print>

namespace coverpp
{
int serve(const ServeOptions& options)
{
    const std::filesystem::path webapp_path{""};

    crow::SimpleApp app;

    CROW_CATCHALL_ROUTE(app)([&] {
        crow::response response;
        response.set_static_file_info(webapp_path.u8string());
        return "Foo";
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
