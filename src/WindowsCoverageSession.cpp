#include "WindowsCoverageSession.hpp"
#include "com_utils.hpp"
#include "file_util.hpp"

#include <print>

namespace coverpp::windows
{
using detail::get_dword;
using detail::get_dword_r;
using detail::get_single_item;
using detail::get_string;

WindowsCoverageSession::WindowsCoverageSession(CoverageParams params)
    : m_params{std::move(params)}, m_dia{m_params.debug_info}
{

}

std::generator<std::pair<const std::filesystem::path&, IDiaLineNumber&>> WindowsCoverageSession::enum_source_lines()
{
    // TODO: For each source file, ensure that its Last Modified timestamp is <= timestamp of exe

    auto dia_source_files = m_dia.enum_source_files();

    COVERPP_FOR_EACH_COM_ITEM(IDiaSourceFile, dia_source_file, *dia_source_files)
    {
        std::filesystem::path file = get_string(dia_source_file, &IDiaSourceFile::get_fileName);
        if (!should_trace(file))
        {
            continue;
        }

        wil::com_ptr<IDiaEnumSymbols> dia_enum_compilands;
        THROW_IF_FAILED(dia_source_file->get_compilands(dia_enum_compilands.put()));

        COVERPP_FOR_EACH_COM_ITEM(IDiaSymbol, dia_compiland, *dia_enum_compilands)
        {
            wil::com_ptr<IDiaEnumLineNumbers> dia_enum_line_numbers;
            THROW_IF_FAILED(m_dia.session().findLines(
                dia_compiland.get(), dia_source_file.get(), dia_enum_line_numbers.put()
            ));

            COVERPP_FOR_EACH_COM_ITEM(IDiaLineNumber, dia_line_number, *dia_enum_line_numbers)
            {
                co_yield {file, *dia_line_number};
            }
        }
    }
}

CoverageSink WindowsCoverageSession::collect_source_lines()
{
    CoverageSink sink;
    for (const auto& [file, dia_line_number] : enum_source_lines())
    {
        sink.track_coverage(
            Tracepoint{
                .source_file = file,
                .tracepoint = {
                    .lineBegin = get_dword_r(dia_line_number, &IDiaLineNumber::get_lineNumber),
                    .columnBegin = get_dword_r(dia_line_number, &IDiaLineNumber::get_columnNumber),
                    .lineEnd = get_dword_r(dia_line_number, &IDiaLineNumber::get_lineNumberEnd),
                    .columnEnd = get_dword_r(dia_line_number, &IDiaLineNumber::get_columnNumberEnd),
                },
            }
        );
    }
    return sink;
}

std::optional<VirtualAddress> WindowsCoverageSession::find_entrypoint()
{
    wil::com_ptr<IDiaSymbol> dia_global_scope;
    THROW_IF_FAILED(m_dia.session().get_globalScope(dia_global_scope.put()));

    for (LPCOLESTR name : {L"main", L"wmain", L"WinMain", L"wWinMain"})
    {
        wil::com_ptr<IDiaEnumSymbols> dia_enum_main;
        THROW_IF_FAILED(dia_global_scope->findChildren(
            SymTagEnum::SymTagFunction, name, nsfCaseSensitive, dia_enum_main.put()
        ));
        if (auto dia_main = get_single_item<IDiaSymbol>(*dia_enum_main))
        {
            return VirtualAddress{get_dword(dia_main, &IDiaSymbol::get_virtualAddress)};
        }
    }

    return std::nullopt;
}

static std::optional<std::filesystem::path> get_file_by_line_numbers(IDiaEnumLineNumbers& line_numbers)
{
    THROW_IF_FAILED(line_numbers.Reset());

    DWORD celt;
    wil::com_ptr<IDiaLineNumber> line_number;
    THROW_IF_FAILED(line_numbers.Next(1, line_number.put(), &celt));
    if (celt != 1)
    {
        return std::nullopt;
    }

    wil::com_ptr<IDiaSourceFile> src_file;
    THROW_IF_FAILED(line_number->get_sourceFile(src_file.put()));

    return get_string(src_file, &IDiaSourceFile::get_fileName);
}

void WindowsCoverageSession::trace(CoverageSink& sink, VirtualAddress va)
{
    const auto tracepoint = resolve_tracepoint(va);

    if (m_params.verbosity >= 5)
    {
        std::println("Breakpoint hit at {}",
                     tracepoint
                         ? std::format("{}:{} (address {})",
                                       tracepoint->source_file.u8string(),
                                       tracepoint->tracepoint.lineBegin,
                                       va)
                         : std::format("address {}", va));
    }

    if (!tracepoint || !should_trace(tracepoint->source_file))
    {
        return;
    }

    sink.track_coverage(*tracepoint);
}

std::optional<Tracepoint>
WindowsCoverageSession::resolve_tracepoint(VirtualAddress va)
{
    wil::com_ptr<IDiaEnumLineNumbers> line_numbers;
    THROW_IF_FAILED(m_dia.session().findLinesByVA(va.value, 1, line_numbers.put()));

	auto const file = get_file_by_line_numbers(*line_numbers);
	if (!file)
	{
		return std::nullopt;
	}

	auto line_number = get_single_item<IDiaLineNumber>(*line_numbers);
	if (!line_number)
	{
		// TODO: This happens sometimes. We probably need to return multiple tracepoints?
		return std::nullopt;
	}

	return Tracepoint{*file,
	                  {
	                      .lineBegin   = get_dword(line_number, &IDiaLineNumber::get_lineNumber),
	                      .columnBegin = get_dword(line_number, &IDiaLineNumber::get_columnNumber),
	                      .lineEnd     = get_dword(line_number, &IDiaLineNumber::get_lineNumberEnd),
	                      .columnEnd   = get_dword(line_number, &IDiaLineNumber::get_columnNumberEnd),
	                  }};
}

DiaAccessor& WindowsCoverageSession::dia()
{
    return m_dia;
}

bool WindowsCoverageSession::should_trace(std::filesystem::path const& source_file) const
{
	if (!m_params.source_dir.empty() && !detail::path_is_subpath_of(source_file, m_params.source_dir))
	{
		return false;
	}

	if (std::regex_search(source_file.generic_string(), m_params.exclude_source_files_regex))
	{
		return false;
	}

	return true;
}
} // namespace coverpp::windows
