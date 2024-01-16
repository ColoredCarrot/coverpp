#include "WindowsCoverageSession.hpp"
#include "com_utils.hpp"
#include "file_util.hpp"

namespace coverpp::windows
{
template<typename T>
static std::string get_string(const wil::com_ptr<T>& com, HRESULT (T::* f)(BSTR*))
{
    BSTR bs;
    THROW_IF_FAILED(((*com).*f)(&bs));
    return detail::windows::bstr_to_utf8_string(bs);
}

template<std::integral V, typename T>
static V get_dword(const wil::com_ptr<T>& com, HRESULT (T::* f)(V*))
{
    V v;
    THROW_IF_FAILED(((*com).*f)(&v));
    return v;
}

template<typename TItem, typename TEnum>
static wil::com_ptr<TItem> get_single_item(TEnum& enumeration)
{
    LONG count;
    THROW_IF_FAILED(enumeration.get_Count(&count));
    if (count != 1)
    {
        return {};
    }

    wil::com_ptr<TItem> item;
    THROW_IF_FAILED(enumeration.Item(0, item.put()));
    return item;
}


WindowsCoverageSession::WindowsCoverageSession(CoverageParams params)
    : m_params{std::move(params)}, m_dia{m_params.debug_info}
{

}

CoverageSink WindowsCoverageSession::collect_source_lines()
{
    auto dia_source_files = m_dia.enum_source_files();

    coverpp::CoverageSink sink;
    COVERPP_FOR_EACH_COM_ITEM(IDiaSourceFile, dia_source_file, *dia_source_files)
    {
        std::filesystem::path file = get_string(dia_source_file, &IDiaSourceFile::get_fileName);
        if (!detail::path_is_subpath_of(file, m_params.source_dir))
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
                sink.track_coverage(
                    file,
                    {
                        .lineBegin = get_dword(dia_line_number, &IDiaLineNumber::get_lineNumber),
                        .columnBegin = get_dword(dia_line_number, &IDiaLineNumber::get_columnNumber),
                        .lineEnd = get_dword(dia_line_number, &IDiaLineNumber::get_lineNumberEnd),
                        .columnEnd = get_dword(dia_line_number, &IDiaLineNumber::get_columnNumberEnd),
                    }
                );
            }
        }
    }

    return sink;
}

VirtualAddress WindowsCoverageSession::find_entrypoint()
{
    wil::com_ptr<IDiaSymbol> dia_global_scope;
    THROW_IF_FAILED(m_dia.session().get_globalScope(dia_global_scope.put()));

    wil::com_ptr<IDiaEnumSymbols> dia_enum_main;
    THROW_IF_FAILED(dia_global_scope->findChildren(
        SymTagEnum::SymTagFunction, L"main", nsfCaseSensitive, dia_enum_main.put()
    ));
    auto dia_main = get_single_item<IDiaSymbol>(*dia_enum_main);
    if (!dia_main)
    {
        throw std::runtime_error("Could not find main function in PDB");
    }

    return VirtualAddress{get_dword(dia_main, &IDiaSymbol::get_virtualAddress)};
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

std::optional<std::filesystem::path> WindowsCoverageSession::trace(CoverageSink& sink, VirtualAddress va)
{
    wil::com_ptr<IDiaEnumLineNumbers> line_numbers;
    THROW_IF_FAILED(m_dia.session().findLinesByVA(va.value, 1, line_numbers.put()));

    const auto file = get_file_by_line_numbers(*line_numbers);

    if (file && coverpp::detail::path_is_subpath_of(*file, m_params.source_dir))
    {
        auto line_number = get_single_item<IDiaLineNumber>(*line_numbers);
        sink.track_coverage(
            *file,
            {
                .lineBegin = get_dword(line_number, &IDiaLineNumber::get_lineNumber),
                .columnBegin = get_dword(line_number, &IDiaLineNumber::get_columnNumber),
                .lineEnd = get_dword(line_number, &IDiaLineNumber::get_lineNumberEnd),
                .columnEnd = get_dword(line_number, &IDiaLineNumber::get_columnNumberEnd),
            }
        );
    }

    return file;
}
}
