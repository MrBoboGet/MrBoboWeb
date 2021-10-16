project(MBWebsite)
set(PROJECT_NAME "MBWebsite")
##BEGIN MBPM_VARIABLES
set(MBPM_DEPENDENCIES 
	MBPacketManager
	MBCLI
	MBUtility
	MBUnicode
	MBCrypto
	cryptopp
	MrBigInt
	MBParsing
	MrBoboSockets
	MBSystem
	MBMime
	MBSearchEngine
	MrPostOGet
	MrBoboDatabase
	sqlite3
)
set(MBPM_TARGET_EXTPACKET_LIBRARIES )
set(MBPM_TARGET_COMPILE_OPTIONS )
set(MBPM_TARGET_LINK_OPTIONS )
#MBPM_Functions#variables MBPM_DEPENDENCIES
#MBPM_TARGET_EXTPACKET_LIBRARIES
#MBPM_TARGET_COMPILE_OPTIONS
#MBPM_TARGET_LINK_OPTIONS
#Environment variables MBPM_PACKETS_INSTALL_DIRECTORY
macro(MBPM_UpdateLibraries TargetToUpdate LinkType BuildType)
	set(MBPM_TARGET_EXTPACKET_LIBRARIES "")
	foreach(PACKET IN LISTS MBPM_DEPENDENCIES)
		set(NewLibraryName "")
		set(NewLibraryToAdd "$ENV{MBPM_PACKETS_INSTALL_DIRECTORY}/${PACKET}/MBPM_Builds/")
		if(${LinkType} STREQUAL "STATIC")
			set(NewLibraryName "${PACKET}_S")
		elseif(${LinkType} STREQUAL "DYNAMIC")
			set(NewLibraryName "${PACKET}_D")
		endif()
		if(${BuildType} STREQUAL "DEBUG")
			set(NewLibraryName "${NewLibraryName}D")
		elseif(${BuildType} STREQUAL "RELEASE")
			set(NewLibraryName "${NewLibraryName}R")
		endif()
		if(UNIX)
			set(NewLibraryName "lib${NewLibraryName}.")
			if(${LinkType} STREQUAL "STATIC")
				set(NewLibraryName "${NewLibraryName}a")
			else()
				set(NewLibraryName "${NewLibraryName}so")
			endif()
		else()
			if(${LinkType} STREQUAL "STATIC")
				set(NewLibraryName "${NewLibraryName}.lib")
			else()
				set(NewLibraryName "${NewLibraryName}.dll")				
			endif()
		endif()
		set(NewLibraryToAdd "${NewLibraryToAdd}${NewLibraryName}")
		list(APPEND MBPM_TARGET_EXTPACKET_LIBRARIES ${NewLibraryToAdd})
	endforeach()
endmacro()

macro(MBPM_UpdateTargetVariables TargetToUpdate LinkType BuildType)
	MBPM_UpdateLibraries(${TargetToUpdate} ${LinkType} ${BuildType})
endmacro()

macro(MBPM_UpdateOutput TargetToUpdate)
	set_target_properties(${TargetToUpdate} 
	PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/MBPM_Builds/"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/MBPM_Builds/"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/MBPM_Builds/"
	)
	#set_target_properties(${TargetToUpdate} PROPERTIES PREFIX "")
	#set_target_properties(${TargetToUpdate} PROPERTIES SUFFIX "")
	set_target_properties(${TargetToUpdate} PROPERTIES OUTPUT_NAME ${TargetToUpdate})
endmacro()

macro(MBPM_SetTargetCompileOptions TargetToUpdate LinkType BuildType)
	if(UNIX)

	else()
		if(${BuildType} STREQUAL "DEBUG" AND ${LinkType} STREQUAL "STATIC")
			target_compile_options(${TargetToUpdate} PRIVATE /MTd)
		else()
			target_compile_options(${TargetToUpdate} PRIVATE /MT)
		endif()
	endif()
endmacro()

macro(MBPM_LinkTargetPacketLibraries Target)
	target_link_libraries(${Target} ${MBPM_TARGET_EXTPACKET_LIBRARIES})
endmacro()

macro(MBPM_SetTargetIncludeDirectories Target)
	target_include_directories(${Target} PUBLIC 
	"${CMAKE_CURRENT_SOURCE_DIR}/"
	"$ENV{MBPM_PACKETS_INSTALL_DIRECTORY}/"
	"$ENV{MBPM_PACKETS_INSTALL_DIRECTORY}/MBUtility/"
	)
endmacro()

macro(MBPM_ApplyTargetConfiguration TargetToUpdate LinkType BuildType)
	MBPM_UpdateTargetVariables(${TargetToUpdate} ${LinkType} ${BuildType})
	MBPM_UpdateOutput(${TargetToUpdate})
	MBPM_SetTargetCompileOptions(${TargetToUpdate} ${LinkType} ${BuildType})
	MBPM_SetTargetIncludeDirectories(${TargetToUpdate})
endmacro()
##END MBPM_VARIABLES
set(PROJECT_SOURCES 

	"${CMAKE_CURRENT_SOURCE_DIR}//MPGMemeSite.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/operationblipp.css"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/WebsidanStil.css"
)
set(PROJECT_HEADERS 
	"${CMAKE_CURRENT_SOURCE_DIR}//MPGMemeSite.h"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/404.html"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/AudioTest.html"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/DBAdd.html"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/DBEditTemplate.html"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/DBLogin.html"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/DBSite.html"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/DBUpdate.html"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/DBUploadFile.html"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/DBViewFolder.html"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/DBViewTemplate.html"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/DirectFileStreamTemplate.html"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/EmbeddStreamTemplate.html"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/index.htm"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/index3.htm"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/operationblipp.html"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/SearchIndex.html"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/StreamingTest.html"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/TopMenu.html"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/UploadFileTest.html"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/VideoTest.html"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/VideoTestPlain.html"
	"${CMAKE_CURRENT_SOURCE_DIR}//ServerResources/mrboboget.se/HTMLResources/YoutoubeTest.html"
)
set(COMMON_FILES ${PROJECT_SOURCES} ${PROJECT_HEADERS})
set(COMMON_DYNAMIC_LIBRARIES 
)
set(COMMON_STATIC_LIBRARIES 
)
set(COMMON_LIBRARIES ${COMMON_STATIC_LIBRARIES} ${COMMON_DYNAMIC_LIBRARIES})

add_library(MBWebsite_SD STATIC ${COMMON_FILES})
MBPM_ApplyTargetConfiguration("MBWebsite_SD" "STATIC" "DEBUG")
target_compile_features("MBWebsite_SD" PRIVATE cxx_std_17)
target_link_libraries("MBWebsite_SD" ${COMMON_LIBRARIES})

add_library(MBWebsite_SR STATIC ${COMMON_FILES})
MBPM_ApplyTargetConfiguration("MBWebsite_SR" "STATIC" "RELEASE")
target_compile_features("MBWebsite_SR" PRIVATE cxx_std_17)
target_link_libraries("MBWebsite_SR" ${COMMON_LIBRARIES})
