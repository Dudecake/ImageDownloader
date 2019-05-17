cmake_minimum_required(VERSION 3.9)

option(PROFILE "Profile using gperf" OFF)
option(HUNTER "Use Hunter package manager" OFF)

if (${HUNTER})
    include("cmake/HunterGate.cmake")
    HunterGate(
        URL "https://github.com/ruslo/hunter/archive/v0.23.168.tar.gz"
        SHA1 "27c195b680097a1c4e16688a2f2f4b7e22d3ec36"
    )
endif()

project(ImageDownloader)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT supported OUTPUT error)
    if( supported )
        message(STATUS "IPO / LTO enabled")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "IPO / LTO not supported: <${error}>")
    endif()
    add_compile_options("-ftree-vectorize")
endif()

#if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
#    set(FILESYSTEM stdc++fs)
#elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
#    set(FILESYSTEM stdc++fs)
#endif()
set(FILESYSTEM stdc++fs)

if (${HUNTER})
    hunter_add_package(Qt)
    hunter_add_package(spdlog)
endif()

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(spdlog REQUIRED)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

set(SRC "src/main.cpp" "src/mainwindow.cpp" "src/searchdialog.cpp"
        "src/image/image.cpp" "src/image/imagefinder.cpp" "src/image/imageinfo.cpp"
        "src/network/downloadhelper.cpp" "src/network/danbooruworker.cpp" "src/network/konachanworker.cpp"
        "src/network/gelbooruworker.cpp")
set(HEADERS "include/utils.h" "include/logger.h" "include/mainwindow.h" "include/searchdialog.h"
            "include/image/fraction.h" "include/image/image.h" "include/image/imagefinder.h" "include/image/imageinfo.h" "include/image/blacklistimage.h" "include/image/wallpaperimage.h"
            "include/network/downloadhelper.h" "include/network/workerfactory.h" "include/network/imageworker.h" "include/network/danbooruworker.h" "include/network/konachanworker.h" "include/network/gelbooruworker.h"
            "include/nlohmann/json.hpp" "include/cxxopts/cxxopts.hpp")

set(FORMS "src/mainwindow.ui" "src/searchdialog.ui")
set(RESOURCES "resources/imagedownloader.qrc")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")

add_executable(${PROJECT_NAME} ${SRC} ${HEADERS} ${FORMS} ${RESOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
if(PROFILE)
    set(PROFILER "profiler")
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy" DOC "Path to clang-tidy executable")
    if(NOT CLANG_TIDY_EXE)
      message(STATUS "clang-tidy not found.")
    else()
      message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
      set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=*,-clang-analyzer-alpha.*,-android-*,-fuchsia-*,-hicpp-use-auto,-modernize-use-auto,-hicpp-vararg,-cppcoreguidelines-pro-type-vararg,-readability-implicit-bool-conversion,-cppcoreguidelines-pro-bounds-array-to-pointer-decay,-hicpp-no-array-decay")
    endif()

    if(CLANG_TIDY_EXE)
      set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
    endif()
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
target_link_libraries(${PROJECT_NAME} ${PROFILER} ${FILESYSTEM} pthread sqlite3 curl Qt5::Core Qt5::Widgets spdlog::spdlog)
