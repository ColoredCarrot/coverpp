cmake_minimum_required(VERSION 3.27)

function(find_asan_dlls out)
    # The DLLs we need are siblings of ${CMAKE_CXX_COMPILER}
    # This is so incredibly ugly.
    # If we want to make this "better" at any point: https://izzys.casa/2023/09/finding-msvc-with-cmake/

    set(${out})
    foreach (dll "clang_rt.asan_dynamic-x86_64.dll" "clang_rt.asan_dbg_dynamic-x86_64.dll")
        cmake_path(REPLACE_FILENAME CMAKE_CXX_COMPILER "${dll}" OUTPUT_VARIABLE dll_path)
        list(APPEND ${out} "${dll_path}")
    endforeach ()

    message(STATUS "Found MSVC ASAN DLLs: ${${out}}")

    return(PROPAGATE ${out})
endfunction()

function(copy_asan_dlls target)
    if (NOT MSVC)
        return()
    endif ()

    find_asan_dlls(dlls)
    foreach (dll ${dlls})
        add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${dll}" "$<TARGET_FILE_DIR:${target}>"
        )
    endforeach ()
endfunction()
