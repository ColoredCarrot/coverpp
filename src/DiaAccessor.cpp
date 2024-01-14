#include "DiaAccessor.hpp"

namespace coverpp::windows
{
using DllGetClassObject_t = HRESULT(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID FAR* ppv);

static wil::unique_hmodule load_library(const std::filesystem::path& path)
{
    return wil::unique_hmodule(THROW_LAST_ERROR_IF_NULL(LoadLibraryW(path.c_str())));
}

static DllGetClassObject_t* get_DllGetClassObject_proc(const wil::unique_hmodule& library)
{
    return reinterpret_cast<DllGetClassObject_t*>(THROW_LAST_ERROR_IF_NULL(
        GetProcAddress(library.get(), "DllGetClassObject")));
}

static wil::com_ptr<IDiaDataSource> get_dia_data_source(const wil::unique_hmodule& dll)
{
    /*
     * High-level procedure:
     *  1. Call DllGetClassObject with IID_IClassFactory
     *  2. Call cf->CreateInstance
     * See https://stackoverflow.com/a/2187454
     */

    const auto DllGetClassObject_proc = get_DllGetClassObject_proc(dll);

    wil::com_ptr<IClassFactory> cf;
    THROW_IF_FAILED(DllGetClassObject_proc(CLSID_DiaSource, IID_IClassFactory, cf.put_void()));

    wil::com_ptr<IDiaDataSource> dia_data_source;
    cf->CreateInstance(nullptr, IID_IDiaDataSource, dia_data_source.put_void());

    return dia_data_source;
}


DiaAccessor::DiaAccessor(const std::filesystem::path& pdb)
    : m_library(load_library("msdia140.dll")), m_data_source(get_dia_data_source(m_library))
{
    THROW_IF_FAILED(m_data_source->loadDataFromPdb(pdb.c_str()));

    THROW_IF_FAILED(m_data_source->openSession(m_session.put()));
}

IDiaSession& DiaAccessor::session()
{
    return *m_session;
}

wil::com_ptr<IDiaEnumSourceFiles> DiaAccessor::enum_source_files()
{
    // See https://learn.microsoft.com/en-us/visualstudio/debugger/debug-interface-access/idiaenumsourcefiles?view=vs-2022

    wil::com_ptr<IDiaEnumTables> dia_tables;
    THROW_IF_FAILED(m_session->getEnumTables(dia_tables.put()));

    wil::com_ptr<IDiaEnumSourceFiles> dia_source_files;
    COVERPP_FOR_EACH_COM_ITEM(IDiaTable, dia_table, *dia_tables)
    {
        const HRESULT hr = dia_table->QueryInterface(IID_IDiaEnumSourceFiles, dia_source_files.put_void());
        if (hr == S_OK)
        {
            break;
        }
        (void) dia_source_files.detach();
    }

    return dia_source_files;
}
}
