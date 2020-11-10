cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

project(BasicChatCmake CXX)

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################
if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x64")
endif()
message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

if(NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64"
     OR "${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86"))
    message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
endif()

################################################################################
# Global configuration types
################################################################################
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    CACHE STRING "" FORCE
)
#set_property(TARGET tgt PROPERTY CXX_STANDARD 20)
################################################################################
# Global compiler options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "")
endif()

################################################################################
# Global linker options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_EXE_LINKER_FLAGS "")
    set(CMAKE_MODULE_LINKER_FLAGS "")
    set(CMAKE_SHARED_LINKER_FLAGS "")
    set(CMAKE_STATIC_LINKER_FLAGS "")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS}")
endif()

################################################################################
# Nuget packages function stub.
################################################################################
function(use_package TARGET PACKAGE VERSION)
    message(WARNING "No implementation of use_package. Create yours. "
                    "Package \"${PACKAGE}\" with version \"${VERSION}\" "
                    "for target \"${TARGET}\" is ignored!")
endfunction()

################################################################################
# Common utils
################################################################################
include(CMake/Utils.cmake)

################################################################################
# Additional Global Settings(add specific info there)
################################################################################
include(CMake/GlobalSettingsInclude.cmake OPTIONAL)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Sub-projects
################################################################################



























set(PROJECT_NAME BasicChatCmake)

################################################################################
# Source groups
################################################################################
set(no_group_source_files
    "Crawler/ExampleHeader.txt"
    "Crawler/FungerandeCrawlnna.txt"
    "WebsiteResources/index.htm"
    "WebsiteResources/RedirectToSearch.js"
    "WebsiteResources/WebsidanStil.css"
)
source_group("" FILES ${no_group_source_files})

set(Header_Files
    "Crawler/Crawler.h"
    "Crawler/StringGrejer.h"
    "MrBigInt/MrBigInt.h"
    "MrBoboChatt/MrBoboChatt.h"
    "MrBoboSockets.h"
    "MrPostOGet/Asn1Handlers.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MrPostOGet/MrPostOGet.h"
    "MrPostOGet/SearchEngineImplementation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MrPostOGet/TLSHandler.h"
    "SearchEngine/MBSearchEngine.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "BasicChat/BasicChatMain.cpp"
    "BasicChat/ClientSide.h"
    "BasicChat/ServerSide.h"
    "ClientSideWinsockSample.h"
    "BasiChatCmake.cpp"
    "MrPostOGet/TLSHandler.cpp"
    "ServersideWinsockSample.h"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${no_group_source_files}
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE BasicChatCmake)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
endif()
################################################################################
# Include directories
################################################################################
#testar att ha olika beroende på om det är windows eller inte
if(UNIX)
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../Program Files %28x86%29/Windows Kits/10/Include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/BasicChat;"
        "${CMAKE_CURRENT_SOURCE_DIR}/Crawler;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../MyLibrary;"
        #"${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/gmp-6.2.0;"
        #"${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/gmp-6.2.0/mpz;"
        #"${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_gc/x64/Debug;"
        #"${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_cxx/x64/Debug;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/mpir-3.0.0/lib/Linux"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/SingleHeaderSha256;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/AES"
    )
else()
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../Program Files %28x86%29/Windows Kits/10/Include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/BasicChat;"
        "${CMAKE_CURRENT_SOURCE_DIR}/Crawler;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../MyLibrary;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/gmp-6.2.0;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/gmp-6.2.0/mpz;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_gc/x64/Debug;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_cxx/x64/Debug;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/SingleHeaderSha256;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/AES"
    )
endif()
#[[
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../Program Files %28x86%29/Windows Kits/10/Include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/BasicChat;"
        "${CMAKE_CURRENT_SOURCE_DIR}/Crawler;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../MyLibrary;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/gmp-6.2.0;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/gmp-6.2.0/mpz;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_gc/x64/Debug;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_cxx/x64/Debug;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/SingleHeaderSha256;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/AES"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../Program Files %28x86%29/Windows Kits/10/Include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/BasicChat;"
        "${CMAKE_CURRENT_SOURCE_DIR}/Crawler;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../MyLibrary;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/gmp-6.2.0;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/gmp-6.2.0/mpz;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_gc/x64/Debug;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_cxx/x64/Debug;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/SingleHeaderSha256;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/AES"
    )
else()
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../Program Files %28x86%29/Windows Kits/10/Include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/BasicChat;"
        "${CMAKE_CURRENT_SOURCE_DIR}/Crawler;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../MyLibrary;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/gmp-6.2.0;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/gmp-6.2.0/mpz;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_gc/x64/Debug;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_cxx/x64/Debug;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/SingleHeaderSha256;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ExternaLibrarys/AES"
    )
endif()
]]
################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "WIN32;"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
endif()
##borde fixa för linux

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy
            >
            /permissive-;
            /std:c++latest;
            /sdl-;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy
            >
            /permissive-;
            /std:c++latest;
            /sdl-;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /INCREMENTAL
            >
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            /DEBUG;
            /SUBSYSTEM:CONSOLE
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /INCREMENTAL
            >
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            /DEBUG;
            /SUBSYSTEM:CONSOLE
        )
    endif()
endif()

################################################################################
# Dependencies
################################################################################
#windows
if(UNIX)
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        #"Ws2_32;"
        #"libmpir.so.23.0.3"
        #"libmpirxx.so.8.4.3"
        "libmpir.a"
        "libmpirxx.a"
    )
else()
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "Ws2_32;"
        "mpir;"
        "mpirxx"
    )
endif()
#[[
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "Ws2_32;"
        "mpir;"
        "mpirxx"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "Ws2_32;"
        "mpir;"
        "mpirxx"
    )
elseif(UNIX)
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        #"Ws2_32;"
        "libmpir.so.23.0.3"
        "libmpirxx.so.8.4.3"
    )
endif()
]]
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")
if(UNIX)
    target_link_directories(${PROJECT_NAME} PRIVATE
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/gmp-6.2.0/.libs;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/gmp-6.2.0/mpz/.libs;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/gmp-6.2.0;"
        "C:/MinGW/lib/gcc/mingw32/9.2.0;"
        "C:/MinGW/lib;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_cxx/x64/Debug;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_gc/x64/Debug"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/mpir-3.0.0/lib/Linux"
    )
else()
    target_link_directories(${PROJECT_NAME} PRIVATE
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/gmp-6.2.0/.libs;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/gmp-6.2.0/mpz/.libs;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/gmp-6.2.0;"
        "C:/MinGW/lib/gcc/mingw32/9.2.0;"
        "C:/MinGW/lib;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_cxx/x64/Debug;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_gc/x64/Debug"
    )
endif()
#[[
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_link_directories(${PROJECT_NAME} PRIVATE
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/gmp-6.2.0/.libs;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/gmp-6.2.0/mpz/.libs;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/gmp-6.2.0;"
        "C:/MinGW/lib/gcc/mingw32/9.2.0;"
        "C:/MinGW/lib;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_cxx/x64/Debug;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_gc/x64/Debug"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_link_directories(${PROJECT_NAME} PRIVATE
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/gmp-6.2.0/.libs;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/gmp-6.2.0/mpz/.libs;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/gmp-6.2.0;"
        "C:/MinGW/lib/gcc/mingw32/9.2.0;"
        "C:/MinGW/lib;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_cxx/x64/Debug;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_gc/x64/Debug"
    )
else()
    #default om inget annat är specificerat
    target_link_directories(${PROJECT_NAME} PRIVATE
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/gmp-6.2.0/.libs;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/gmp-6.2.0/mpz/.libs;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/gmp-6.2.0;"
        "C:/MinGW/lib/gcc/mingw32/9.2.0;"
        "C:/MinGW/lib;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_cxx/x64/Debug;"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/mpir-3.0.0/build.vc15/lib_mpir_gc/x64/Debug"
        "C:/Users/emanu/Desktop/Program/C++/ExternaLibrarys/mpir-3.0.0/lib/Linux"
    )
endif()
]]

