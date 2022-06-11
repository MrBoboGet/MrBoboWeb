project(MBSearchEngine)
set(PROJECT_NAME "MBSearchEngine")
##BEGIN MBPM_VARIABLES
set(MBPM_DEPENDENCIES 
	MrPostOGet
	MBSystem
	MBParsing
	MBUtility
	MBMime
	MBUnicode
	MrBoboSockets
	MrBigInt
	MBCrypto
	cryptopp
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

	"${CMAKE_CURRENT_SOURCE_DIR}//MBSearchEngine.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}//MBSearchEngineCLIClass.cpp"
	#"${CMAKE_CURRENT_SOURCE_DIR}//MBSearchEngineCLIMain.cpp"
)
set(PROJECT_HEADERS 
	"${CMAKE_CURRENT_SOURCE_DIR}//BooleanParse.h"
	"${CMAKE_CURRENT_SOURCE_DIR}//MBISave.h"
	"${CMAKE_CURRENT_SOURCE_DIR}//MBISaveIndexSpec.h"
	"${CMAKE_CURRENT_SOURCE_DIR}//MBRecursiveDirectoryIterator.h"
	"${CMAKE_CURRENT_SOURCE_DIR}//MBSearchEngine.h"
	"${CMAKE_CURRENT_SOURCE_DIR}//MBSearchEngineCLIClass.h"
)
set(EXECUTABLE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/MBSearchEngineCLIMain.cpp")

set(COMMON_FILES ${PROJECT_SOURCES} ${PROJECT_HEADERS})
set(COMMON_DYNAMIC_LIBRARIES 
)
set(COMMON_STATIC_LIBRARIES 
)
if(WIN32)
else()
	list(APPEND "COMMON_STATIC_LIBRARIES" "stdc++fs")
endif()
set(COMMON_LIBRARIES ${COMMON_STATIC_LIBRARIES} ${COMMON_DYNAMIC_LIBRARIES})

add_library(MBSearchEngine STATIC ${COMMON_FILES})
MBPM_ApplyConfiguration("MBSearchEngine")
target_compile_features("MBSearchEngine" PRIVATE cxx_std_17)

add_executable("mbse" ${COMMON_FILES} ${EXECUTABLE_FILES})
MBPM_ApplyConfiguration("mbse")
target_compile_features("mbse" PRIVATE cxx_std_17)
target_link_libraries("mbse" ${COMMON_LIBRARIES})

