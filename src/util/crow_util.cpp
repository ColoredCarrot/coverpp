#include "crow_util.hpp"

#include "console_color.hpp"
#include "../file_util.hpp"
#include "../util/open_browser.hpp"

#include <print>

namespace coverpp::detail
{
crow::response make_file_response(std::filesystem::path const& file, std::string_view content_type)
{
	if (!std::filesystem::is_regular_file(file))
	{
		return crow::response{404, "text/plain", "404 Not Found"};
	}

	auto content = read_file_binary(file);
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

void run_server(crow::SimpleApp& app, int port, bool open)
{
	auto future = app.port(port).concurrency(2).loglevel(crow::LogLevel::Warning).run_async();

	app.wait_for_server_start();

	if (open)
	{
		open_browser_at_url(std::format("http://localhost:{}", port));
	}

	std::println("Listening at http://localhost:{}", port);
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
			open_browser_at_url(std::format("http://localhost:{}", port));
		}
	}

	app.stop();

	future.wait_for(std::chrono::seconds{30});
}
} // namespace coverpp::detail
