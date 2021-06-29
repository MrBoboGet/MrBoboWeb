cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

project(BasicChatCmake CXX)

#straight copypastad för att se om jag kunde fixa ett problem med threads på arms
#set(CMAKE_THREAD_LIBS_INIT "-lpthread")
#3set(CMAKE_HAVE_THREADS_LIBRARY 1)
#set(CMAKE_USE_WIN32_THREADS_INIT 0)
#set(CMAKE_USE_PTHREADS_INIT 1)
##set(THREADS_PREFER_PTHREAD_FLAG ON)
#slut på copypastad eventuellt hack

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
















#Global variables
set(EXTLIBS_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../MBExternalDependencies/"
)


set(PROJECT_NAME BasicChatCmake)


#globala linker settings


################################################################################
# Source groups
################################################################################
set(no_group_source_files
    "Crawler/ExampleHeader.txt"
    "Crawler/FungerandeCrawlnna.txt"
    #"WebsiteResources/index.htm"
    #"WebsiteResources/RedirectToSearch.js"
    #"WebsiteResources/WebsidanStil.css"
)
source_group("" FILES ${no_group_source_files})

set(EXTLIBS_HEADER_FILES
    "${EXTLIBS_DIR}/SQLite/sqlite3ext.h"
    "${EXTLIBS_DIR}/SQLite/sqlite3.h"
    "${EXTLIBS_DIR}/cryptopp/cryptlib.h"
    #"${EXTLIBS_DIR}/tinyxml2/tinyxml2.h"
    #"${EXTLIBS_DIR}/ffmpeg/libavutil/avconfig.h"
)
set(EXTLIBS_SOURCE_FILES
    #"${EXTLIBS_DIR}/SQLite/sqlite3.c"
    #"${EXTLIBS_DIR}/SQLite/shell.c"
    #"${EXTLIBS_DIR}/tinyxml2/tinyxml2.cpp"
)

set(Header_Files
    "Crawler/Crawler.h"
    "Crawler/StringGrejer.h"
    "MrBigInt/MrBigInt.h"
    "MrBoboChatt/MrBoboChatt.h"
    "MrBoboSockets/MrBoboSockets.h"
    "MrPostOGet/Asn1Handlers.h"
    "MrPostOGet/MBHTMLParser.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MrPostOGet/MrPostOGet.h"
    "MrPostOGet/SearchEngineImplementation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MrPostOGet/TLSHandler.h"
    "SearchEngine/MBSearchEngine.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../MBExternalDependencies/Hash/src/sha1.h;"
    "MrBoboDatabase/MPGMemeSite.h"
    "MrBoboMedia/MBMedia.h"
    ${EXTLIBS_HEADER_FILES}
    #search engine
    "MBSearchEngine/MBSearchEngine.h"
    "MBSearchEngine/MBUnicode.h"
    "MBSearchEngine/MBUnicodeMacros.h"
    "Crawler/MBCrawler.h"
    "MBDNSHandler/MBDNSHandler.h"
    "Crawler/MBCrawlerSite.h"
    "MBMime/MBMime.h"
    "MrBoboMail/MrBoboMail.h"
    "MBCrypto/MBCrypto.h"

)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "MrBoboSockets/MrBoboSockets.cpp"
    "BasicChat/BasicChatMain.cpp"
    "BasiChatCmake.cpp"
    "MrPostOGet/TLSHandler.cpp"
    "MrBoboDatabase/MrBoboDatabase.cpp"
    "MrBoboDatabase/MPGMemeSite.cpp"
    "MrBoboMedia/MBMedia.cpp"
    "MBSearchEngine/MBSearchEngine.cpp"
    "MBSearchEngine/MBUnicode.cpp"
    "Crawler/MBCrawler.cpp"
    "MBDNSHandler/MBDNSHandler.cpp"
    "MBMime/MBMime.cpp"
    "MrBoboMail/MrBoboMail.cpp"
    "MBCrypto/MBCrypto.cpp"
    ${EXTLIBS_SOURCE_FILES}
    #"${EXTLIBS_DIR}/SQLite/sqlite3.c"
)
message(${Source_Files})
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${no_group_source_files}
    ${Header_Files}
    ${Source_Files}
)

#MBSE grejer
set (MB_STANDARD_LIBRARY_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../MyLibrary/;"
)

set (MBSE_HEADERS
    "MBSearchEngine/MBSearchEngine.h"
    "MBSearchEngine/MBUnicode.h"
    "MBSearchEngine/MBUnicodeMacros.h"
    "MBSearchEngine/MBUnicode.h"
    "MBSearchEngine/MBISaveIndexSpec.h"
    "MBSearchEngine/MBUnicodeMacros.h"
    "MBSearchEngine/MBSearchEngineCLIClass.h"
    "MBSearchEngine/MBRecursiveDirectoryIterator.h"
)
set (MBSE_SOURCES
    "MBSearchEngine/MBSearchEngine.cpp"
    "MBSearchEngine/MBSearchEngineCLIMain.cpp"
    "MBSearchEngine/MBSearchEngineCLIClass.cpp"
    "MBSearchEngine/MBUnicode.cpp"
    #TODO flytta error handlingen till MB_STANDARD_LIBRARY
    "MBErrorHandling.cpp"
)
set (MBSE_ALL_FILES
   ${MBSE_HEADERS}
   ${MBSE_SOURCES}
)
add_executable("mbse" ${MBSE_ALL_FILES})
target_compile_features("mbse" PRIVATE cxx_std_17)
target_include_directories("mbse" PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/.;"
    "${MB_STANDARD_LIBRARY_DIR}"
)

#hokus pokus

#use_props("mbse" "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
#target_compile_options("mbse" PRIVATE /MTd)
#target_compile_options("mbse" PRIVATE
#    $<$<CXX_COMPILER_ID:MSVC>:/Zi>
#    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-g>
#    )
#if(MSVC)
#    target_link_options("mbse" PUBLIC /DEBUG)
#    target_link_options("mbse" PUBLIC /Zi)
#endif()
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions("mbse" PRIVATE
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
endif()
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options("mbse" PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy
            >
            /permissive-;
            /std:c++latest;
            /sdl-;
            /W3;
            /Zi;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options("mbse" PRIVATE
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
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES} "MrBoboChatt/MBChatConnection.h" "MrBoboChatt/MBCProtocols.h" "MrBoboChatt/MBCProtocols.cpp" "MrBoboChatt/MBChatConnection.cpp" "MBErrorHandling.h" "MBErrorHandling.cpp"  "MrBoboChatt/MrBoboChatt.cpp" "MBCertificateHandler/MBCertificateHandler.h")


target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)


#set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}../../")
#target_link_libraries(${PROJECT_NAME} "sqlite3")

#target_sources(${PROJECT_NAME} PUBLIC ${EXTLIBS_SOURCE_FILES})

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
target_include_directories(${PROJECT_NAME} PUBLIC
    #"${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../Program Files %28x86%29/Windows Kits/10/Include;"
    "${EXTLIBS_DIR}/ffmpeg/"
    "${CMAKE_CURRENT_SOURCE_DIR}/.;"
    "${CMAKE_CURRENT_SOURCE_DIR}/BasicChat;"
    "${CMAKE_CURRENT_SOURCE_DIR}/Crawler;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../MyLibrary/;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../MrBoboLibrary/;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../MBExternalDependencies/PicoSHA2/;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../MBExternalDependencies/;"
    #"${CMAKE_CURRENT_SOURCE_DIR}/../MBExternalDependencies/plusaes/include/plusaes/"
)
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
set (FFMPEG_LIBS
    "${EXTLIBS_DIR}/ffmpeg/libavcodec/libavcodec.a"
    "${EXTLIBS_DIR}/ffmpeg/libavdevice/libavdevice.a"
    "${EXTLIBS_DIR}/ffmpeg/libavfilter/libavfilter.a"
    "${EXTLIBS_DIR}/ffmpeg/libavformat/libavformat.a"
    "${EXTLIBS_DIR}/ffmpeg/libavutil/libavutil.a"
    "${EXTLIBS_DIR}/ffmpeg/libswresample/libswresample.a"
    "${EXTLIBS_DIR}/ffmpeg/libswscale/libswscale.a"
)

if(UNIX)
    set(ADDITIONAL_LIBRARY_DEPENDENCIES 
        "stdc++fs";
        "${EXTLIBS_DIR}/SQLite/sqlite3.a"
        "${EXTLIBS_DIR}/cryptopp/libcryptopp.a"
        "dl"
        #"${FFMPEG_LIBS}"
    )
else()
    set(ADDITIONAL_LIBRARY_DEPENDENCIES 
        "Ws2_32"
        #"ws2_32.lib"
        "Secur32.lib"
        "Bcrypt.lib"
        "Mfplat.lib"
        "Mfuuid.lib"
        "Strmiids.lib"
        "${EXTLIBS_DIR}/SQLite/sqlite3.lib"
        #"${EXTLIBS_DIR}/cryptopp/cryptlib_D.lib"
        #"${EXTLIBS_DIR}/cryptopp/cryptlib.lib"
        #"${FFMPEG_LIBS}"
    )
endif()

#mina link options
if(MSVC)
    message("Build type: ${CMAKE_BUILD_TYPE}")
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        message("Bygger Debug Build")    
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
        target_compile_options(${PROJECT_NAME} PRIVATE /MTd)
        target_link_options(${PROJECT_NAME} PRIVATE /INCREMENTAL:NO /NODEFAULTLIB:MSVCRT)
        list(APPEND ADDITIONAL_LIBRARY_DEPENDENCIES "${EXTLIBS_DIR}/cryptopp/cryptlib_D.lib")
    else()
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
        target_compile_options(${PROJECT_NAME} PRIVATE /MT)
        target_link_options(${PROJECT_NAME} PRIVATE /INCREMENTAL:NO /NODEFAULTLIB:MSVCRT)
        list(APPEND ADDITIONAL_LIBRARY_DEPENDENCIES "${EXTLIBS_DIR}/cryptopp/cryptlib.lib")
    endif()
    message("Additional librarys: ${ADDITIONAL_LIBRARY_DEPENDENCIES}")
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")
if(UNIX)
    target_link_directories(${PROJECT_NAME} PRIVATE

    )
else()
    target_link_directories(${PROJECT_NAME} PRIVATE

    )
endif()
