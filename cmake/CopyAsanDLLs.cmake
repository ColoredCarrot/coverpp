function(copy_asan_dlls target)
    set(MSVC_DLL_DIR [=[D:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.38.33130\bin\Hostx64\x64\]=])
    set(MSVC_DLL_LIST
            "${MSVC_DLL_DIR}clang_rt.asan_dynamic-x86_64.dll"
            "${MSVC_DLL_DIR}clang_rt.asan_dbg_dynamic-x86_64.dll"
    )
    foreach (dll ${MSVC_DLL_LIST})
        add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${dll}" "$<TARGET_FILE_DIR:${target}>"
        )
    endforeach ()
endfunction()
