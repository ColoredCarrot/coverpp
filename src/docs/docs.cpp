#include "docs.hpp"

#include "../util/open_browser.hpp"
#include "../util/console_color.hpp"
#include "../util/crow_util.hpp"

#include <crow.h>
#include <print>

namespace coverpp
{
using detail::make_file_response;

int serve_docs(DocsOptions const& options)
{
	std::filesystem::path const docs_path{options.coverpp_install_dir.parent_path() / "docs"};

	if (!std::filesystem::is_directory(docs_path))
	{
		throw std::runtime_error{
		    std::format("Failed to find documentation at {}\nSpecify an alternative installation directory with {}",
		                docs_path.u8string(),
		                styled<ColorBold::gray>("--coverpp-install-dir <dir>"))};
	}

	crow::SimpleApp app;

	CROW_ROUTE(app, "/<path>")([&](std::filesystem::path const& requested_path) {
		for (auto const& path :
		     {docs_path / requested_path, docs_path / requested_path / "index.html", docs_path / "404.html"})
		{
			if (std::filesystem::is_regular_file(path))
			{
				std::string extension = path.extension().string();
				if (extension.starts_with('.'))
				{
					extension.erase(extension.begin());
				}
				return make_file_response(path, crow::response::get_mime_type(extension));
			}
		}

		// Not even the 404 is found
		return crow::response{404, "Not found"};
	});

	CROW_ROUTE(app, "/")([&] { return make_file_response(docs_path / "index.html", "text/html; charset=utf-8"); });

	detail::run_server(app, options.port, options.open);

	return 0;
}
} // namespace coverpp
