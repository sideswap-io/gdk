include(GNUInstallDirs)

macro(create_greenaddress_target)
    add_library(greenaddress $<TARGET_OBJECTS:greenaddress-objects>)
    set_target_properties(greenaddress PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        PUBLIC_HEADER $<TARGET_PROPERTY:greenaddress-objects,PUBLIC_HEADER>
    )
    get_target_property(_gaIncludeDir greenaddress-objects INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(greenaddress INTERFACE ${_gaIncludeDir})
    target_link_libraries(greenaddress PRIVATE
        gdk-rust
        extern::sqlite3
        Microsoft.GSL::GSL
        extern::autobahn-cpp
        msgpackc-cxx
        websocketpp::websocketpp
        nlohmann_json::nlohmann_json
        extern::tor
        libevent::core
        PkgConfig::libsecp256k1
        $<TARGET_NAME_IF_EXISTS:libevent::pthreads>
        Boost::boost
        Boost::log
        Boost::thread
        OpenSSL::SSL
        $<$<PLATFORM_ID:Android>:log>
        ZLIB::ZLIB
        $<$<NOT:$<PLATFORM_ID:Android>>:pthread>
        $<TARGET_NAME_IF_EXISTS:extern::bc-ur>
        $<TARGET_NAME_IF_EXISTS:urc::urc>
    )
    get_target_property(_wallycoreLib PkgConfig::wallycore INTERFACE_LINK_LIBRARIES)
    #cmake 3.24 ==> $<LINK_LIBRARY:WHOLE_ARCHIVE,PkgConfig::wallycore>
    set(_gdkLinkOptions ${GDK_LINK_OPTIONS})
    if(APPLE)
        list(APPEND _gdkLinkOptions "-Wl,-force_load" "SHELL:${_wallycoreLib}")
    else()
        list(APPEND _gdkLinkOptions "LINKER:SHELL:--whole-archive" "SHELL:${_wallycoreLib}" "LINKER:SHELL:--no-whole-archive")
    endif()
    target_link_options(greenaddress PRIVATE "${_gdkLinkOptions}")
    get_library_install_dir(_libInstallDir)
    install(TARGETS greenaddress
        EXPORT "greenaddress-target"
        RUNTIME EXCLUDE_FROM_ALL
        OBJECTS EXCLUDE_FROM_ALL
        ARCHIVE EXCLUDE_FROM_ALL
        LIBRARY DESTINATION ${_libInstallDir}
                COMPONENT gdk-runtime
        OPTIONAL
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gdk
                COMPONENT gdk-dev
                EXCLUDE_FROM_ALL
    )
    install(
        FILES
            ${wallycore_INCLUDE_DIRS}/wally.hpp
            ${wallycore_INCLUDE_DIRS}/wally_address.h
            ${wallycore_INCLUDE_DIRS}/wally_anti_exfil.h
            ${wallycore_INCLUDE_DIRS}/wally_bip32.h
            ${wallycore_INCLUDE_DIRS}/wally_bip38.h
            ${wallycore_INCLUDE_DIRS}/wally_bip39.h
            ${wallycore_INCLUDE_DIRS}/wally_bip85.h
            ${wallycore_INCLUDE_DIRS}/wally_core.h
            ${wallycore_INCLUDE_DIRS}/wally_coinselection.h
            ${wallycore_INCLUDE_DIRS}/wally_crypto.h
            ${wallycore_INCLUDE_DIRS}/wally_descriptor.h
            ${wallycore_INCLUDE_DIRS}/wally_elements.h
            ${wallycore_INCLUDE_DIRS}/wally_map.h
            ${wallycore_INCLUDE_DIRS}/wally_psbt.h
            ${wallycore_INCLUDE_DIRS}/wally_psbt_members.h
            ${wallycore_INCLUDE_DIRS}/wally_script.h
            ${wallycore_INCLUDE_DIRS}/wally_symmetric.h
            ${wallycore_INCLUDE_DIRS}/wally_transaction.h
            ${wallycore_INCLUDE_DIRS}/wally_transaction_members.h
        COMPONENT gdk-dev
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gdk/libwally-core/
        EXCLUDE_FROM_ALL
    )
    get_cmake_install_dir(LIB_CMAKE_INSTALL_DIR ${_libInstallDir})
    install(EXPORT "greenaddress-target"
        COMPONENT gdk-dev
        DESTINATION ${LIB_CMAKE_INSTALL_DIR}/cmake
        NAMESPACE ${PROJECT_NAME}::
        FILE "greenaddress-targets.cmake"
        EXCLUDE_FROM_ALL
    )
endmacro()


macro(create_greenaddressstatic_target)
    add_library(greenaddress-static STATIC $<TARGET_OBJECTS:greenaddress-objects>)
    get_target_property(_gaIncludeDir greenaddress-objects INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(greenaddress-static INTERFACE ${_gaIncludeDir})
    target_link_libraries(greenaddress-static PUBLIC
        PkgConfig::wallycore
        PkgConfig::libsecp256k1
        gdk-rust
        extern::sqlite3
        Microsoft.GSL::GSL
        extern::autobahn-cpp
        msgpackc-cxx
        websocketpp::websocketpp
        nlohmann_json::nlohmann_json
        extern::tor
        libevent::core
        $<$<NOT:$<PLATFORM_ID:Windows>>:libevent::pthreads>
        Boost::boost
        Boost::log
        Boost::thread
        OpenSSL::SSL
        $<$<PLATFORM_ID:Android>:log>
        ZLIB::ZLIB
        $<$<NOT:$<PLATFORM_ID:Android>>:pthread>
        $<TARGET_NAME_IF_EXISTS:extern::bc-ur>
        $<TARGET_NAME_IF_EXISTS:urc::urc>
    )
    target_link_options(greenaddress-static INTERFACE "${GDK_LINK_OPTIONS}")
endmacro()



macro(create_greenaddressfull_target)
    add_library(greenaddress-full STATIC $<TARGET_OBJECTS:greenaddress-objects>)
    set_target_properties(greenaddress-full PROPERTIES
        OUTPUT_NAME greenaddress_full
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        PUBLIC_HEADER $<TARGET_PROPERTY:greenaddress-objects,PUBLIC_HEADER>
    )
    add_dependencies(greenaddress-full gdk-rust)
    get_target_property(_gaIncludeDir greenaddress-objects INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(greenaddress-full INTERFACE ${_gaIncludeDir})
    file(GENERATE OUTPUT archiver.sh INPUT ${CMAKE_SOURCE_DIR}/tools/archiver.sh.gen)
    add_custom_command(TARGET greenaddress-full POST_BUILD
        COMMAND mv $<TARGET_FILE:greenaddress-full> libgreenaddress-partial.a
        COMMAND ./archiver.sh
        COMMAND rm libgreenaddress-partial.a
    )
    target_link_options(greenaddress-full PRIVATE "${GDK_LINK_OPTIONS}")
    get_library_install_dir(_libInstallDir)
    get_cmake_install_dir(LIB_CMAKE_INSTALL_DIR ${_libInstallDir})
    install(TARGETS greenaddress-full
        EXPORT "greenaddress-full-target"
        RUNTIME EXCLUDE_FROM_ALL
        OBJECTS EXCLUDE_FROM_ALL
        LIBRARY EXCLUDE_FROM_ALL
        ARCHIVE DESTINATION ${_libInstallDir}
            COMPONENT gdk-dev
            EXCLUDE_FROM_ALL
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gdk
            COMPONENT gdk-dev
            EXCLUDE_FROM_ALL
        OPTIONAL
    )
    install(EXPORT "greenaddress-full-target"
        COMPONENT gdk-dev
        DESTINATION ${LIB_CMAKE_INSTALL_DIR}/cmake
        NAMESPACE ${PROJECT_NAME}::
        FILE "greenaddress-full-targets.cmake"
        EXCLUDE_FROM_ALL
    )
    install(
        FILES 
            ${wallycore_INCLUDE_DIRS}/wally.hpp
            ${wallycore_INCLUDE_DIRS}/wally_address.h
            ${wallycore_INCLUDE_DIRS}/wally_anti_exfil.h
            ${wallycore_INCLUDE_DIRS}/wally_bip32.h
            ${wallycore_INCLUDE_DIRS}/wally_bip38.h
            ${wallycore_INCLUDE_DIRS}/wally_bip39.h
            ${wallycore_INCLUDE_DIRS}/wally_bip85.h
            ${wallycore_INCLUDE_DIRS}/wally_core.h
            ${wallycore_INCLUDE_DIRS}/wally_coinselection.h
            ${wallycore_INCLUDE_DIRS}/wally_crypto.h
            ${wallycore_INCLUDE_DIRS}/wally_descriptor.h
            ${wallycore_INCLUDE_DIRS}/wally_elements.h
            ${wallycore_INCLUDE_DIRS}/wally_map.h
            ${wallycore_INCLUDE_DIRS}/wally_psbt.h
            ${wallycore_INCLUDE_DIRS}/wally_psbt_members.h
            ${wallycore_INCLUDE_DIRS}/wally_script.h
            ${wallycore_INCLUDE_DIRS}/wally_symmetric.h
            ${wallycore_INCLUDE_DIRS}/wally_transaction.h
            ${wallycore_INCLUDE_DIRS}/wally_transaction_members.h
        COMPONENT gdk-dev
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gdk/libwally-core/
        EXCLUDE_FROM_ALL
    )
endmacro()



