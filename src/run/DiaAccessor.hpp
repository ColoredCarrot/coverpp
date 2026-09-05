#pragma once

#include "types.hpp"

#include <filesystem>

#define NOMINMAX

#include <wil/com.h>
#include <dia2.h>


#define CONCAT(a, b) a##b
#define CONCAT2(a, b) CONCAT(a, b)

#define COVERPP_FOR_EACH_COM_ITEM_(TItem, item, in_enum, celt) \
    wil::com_ptr<TItem> item; DWORD celt; \
    while (THROW_IF_FAILED((in_enum).Next(1, item.put(), &celt)), celt == 1)

#define COVERPP_FOR_EACH_COM_ITEM(TItem, item, in_enum) COVERPP_FOR_EACH_COM_ITEM_(TItem, item, in_enum, CONCAT2(celt, __LINE__))


namespace coverpp::windows
{
class DiaAccessor
{
public:
    explicit DiaAccessor(const std::filesystem::path& pdb);

    IDiaSession& session();

    void set_base_address(VirtualAddress base_address);

    wil::com_ptr<IDiaEnumSourceFiles> enum_source_files();

private:
    wil::unique_hmodule m_library; // needs to outlive any COM objects
    wil::com_ptr<IDiaDataSource> m_data_source;
    wil::com_ptr<IDiaSession> m_session;
};
}
