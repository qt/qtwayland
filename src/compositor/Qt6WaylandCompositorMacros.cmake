function(qt6_generate_wayland_protocol_server_sources target)
    qt_parse_all_arguments(arg "qt6_generate_wayland_protocol_server_sources" "" "" "FILES" ${ARGN})
    get_target_property(target_binary_dir ${target} BINARY_DIR)

    if(NOT TARGET Wayland::Scanner)
        message(FATAL_ERROR "Wayland::Scanner target not found. You might be missing the WaylandScanner CMake package.")
    endif()

    if(NOT TARGET Qt6::qtwaylandscanner)
        message(FATAL_ERROR "qtwaylandscanner executable not found. Most likely there is an issue with your Qt installation.")
    endif()

    foreach(protocol_file IN LISTS arg_FILES)
        get_filename_component(protocol_name "${protocol_file}" NAME_WLE)

        set(waylandscanner_header_output "${target_binary_dir}/wayland-${protocol_name}-server-protocol.h")
        set(waylandscanner_code_output "${target_binary_dir}/wayland-${protocol_name}-protocol.c")
        set(qtwaylandscanner_header_output "${target_binary_dir}/qwayland-server-${protocol_name}.h")
        set(qtwaylandscanner_code_output "${target_binary_dir}/qwayland-server-${protocol_name}.cpp")

        add_custom_command(
            OUTPUT "${waylandscanner_header_output}"
            #TODO: Maybe put the files in ${CMAKE_CURRENT_BINARY_DIR/wayland_generated instead?
            COMMAND Wayland::Scanner --strict --include-core-only server-header < "${protocol_file}" > "${waylandscanner_header_output}"
        )
        add_custom_command(
            OUTPUT "${waylandscanner_code_output}"
            COMMAND Wayland::Scanner --strict --include-core-only public-code < "${protocol_file}" > "${waylandscanner_code_output}"
        )

        # TODO: make this less hacky
        set(wayland_include_dir "")
        #get_target_property(qt_module "${target}" QT_MODULE)
        get_target_property(is_for_module "${target}" INTERFACE_MODULE_HAS_HEADERS)
        if (is_for_module)
            set(wayland_include_dir "QtWaylandCompositor/private")
        endif()

        add_custom_command(
            OUTPUT "${qtwaylandscanner_header_output}"
            COMMAND Qt6::qtwaylandscanner server-header "${protocol_file}" "${wayland_include_dir}" > "${qtwaylandscanner_header_output}"
        )

        add_custom_command(
            OUTPUT "${qtwaylandscanner_code_output}"
            COMMAND Qt6::qtwaylandscanner server-code "${protocol_file}" "${wayland_include_dir}" > "${qtwaylandscanner_code_output}"
        )

        target_sources(${target} PRIVATE
            "${waylandscanner_header_output}"
            "${waylandscanner_code_output}"
            "${qtwaylandscanner_header_output}"
            "${qtwaylandscanner_code_output}"
        )
    endforeach()
    target_include_directories(${target} PRIVATE ${target_binary_dir})
endfunction()

