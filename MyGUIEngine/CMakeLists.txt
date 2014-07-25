set (PROJECTNAME MyGUIEngine)

project( ${PROJECTNAME} )

include(PrecompiledHeader)

include(${PROJECTNAME}.list)

include_directories(include)
if (MYGUI_USE_FREETYPE)
	include_directories(
		${FREETYPE_INCLUDE_DIRS} ${FREETYPE_FT2BUILD_INCLUDE_DIR}
	)
	add_definitions(-DMYGUI_USE_FREETYPE)
endif ()

# Get Svn revision
MACRO(Subversion_GET_REVISION variable)
	FIND_PROGRAM(SVN_EXECUTABLE svnversion
		DOC "subversion command line client")

	EXECUTE_PROCESS(
		COMMAND ${SVN_EXECUTABLE} ${MYGUI_SOURCE_DIR}
		OUTPUT_STRIP_TRAILING_WHITESPACE
		OUTPUT_VARIABLE ${variable})
	#MESSAGE(STATUS  ${MYGUI_SOURCE_DIR} ${variable})
ENDMACRO(Subversion_GET_REVISION)

add_definitions(-DMYGUI_BUILD)

if (MSVC)
	# disable: "no suitable definition provided for explicit template instantiation request"
	# Occurs for no justifiable reason on all #includes of Singleton
	add_definitions(/wd4661)
	add_definitions(/wd4819)
	add_definitions(/wd4275)
endif ()

# setup MyGUIEngine target
add_library(${PROJECTNAME} ${MYGUI_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(${PROJECTNAME} PROPERTIES VERSION ${MYGUI_VERSION} SOVERSION ${MYGUI_VERSION_MAJOR})
if (MYGUI_USE_FREETYPE)
	target_link_libraries(${PROJECTNAME}
		${FREETYPE_LIBRARIES}
	)
endif()

# platform specific dependencies
if (APPLE)
	include_directories(/usr/local/include /usr/include)
	target_link_libraries(${PROJECTNAME} ${Carbon_LIBRARY})
endif ()

if(NOT Build_For_UE)
	# specify a precompiled header to use
	use_precompiled_header(${PROJECTNAME}
	  "${CMAKE_CURRENT_SOURCE_DIR}/include/MyGUI_Precompiled.h"
	  "${CMAKE_CURRENT_SOURCE_DIR}/src/MyGUI_Precompiled.cpp"
	)
endif()

set(${PROJECTNAME}_HEADERS ${HEADER_FILES}) # for Mac OS X
mygui_config_lib(${PROJECTNAME})

# link Carbon on Mac OS X
if (APPLE)
	find_library(CARBON_LIBRARY Carbon)
	target_link_libraries(${PROJECTNAME} ${CARBON_LIBRARY})
endif ()

if(Build_For_UE)
	include_directories( ${freetype_Include_Dir} )
endif()

# install MyGUIEngine headers
install(FILES ${HEADER_FILES}
  DESTINATION include/MYGUI
)
