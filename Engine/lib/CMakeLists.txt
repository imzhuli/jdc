project(${libname})

# dependencies:

# enable specific macors by system type:
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	find_package(glfw3 CONFIG REQUIRED)
	set(GLFW3_LIBRARIES glfw)
	set(PLATFORM_LIBRARIES)
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(PLATFORM_LIBRARIES)
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
	find_library(COCOA_LIBRARY Cocoa REQUIRED)
	find_library(IOKIT_LIBRARY IOKit REQUIRED)
	set(PLATFORM_LIBRARIES ${IOKIT_LIBRARY} ${COCOA_LIBRARY})
endif()

# local dependencies
include_directories(
	../lib/include/
	${X_INCLUDE_DIRS}
	)
link_directories(
	${X_LIBRARY_DIRS}
	)

set(shared_lib_dependency_private
	${X_LIBRARIES}
	${PLATFORM_LIBRARIES}
	${cpp}
	)
set(shared_lib_dependency_public)

aux_source_directory(./src/                          X_GAME_SOURCE)
aux_source_directory(./src/base                      X_GAME_SOURCE)
aux_source_directory(./src/debug                     X_GAME_SOURCE)
aux_source_directory(./src/decompiler                X_GAME_SOURCE)
aux_source_directory(./src/decompiler/converter      X_GAME_SOURCE)
aux_source_directory(./src/class_file                X_GAME_SOURCE)
aux_source_directory(./src/syntax                    X_GAME_SOURCE)

if (X_GAME_OPTION_STATIC)
#static lib:
	add_library(${libname}                   ${X_GAME_SOURCE})
	set_target_properties(${libname}         PROPERTIES VERSION 1.0)
	target_link_libraries(${libname}         PRIVATE ${shared_lib_dependency_private})
	target_link_libraries(${libname}         PUBLIC ${shared_lib_dependency_public})
	target_compile_definitions(${libname}    PRIVATE X_GAME_OPTION_STATIC)

else()
#dynamic lib:
	add_library(${libname} SHARED            ${X_GAME_SOURCE})
	set_target_properties(${libname}         PROPERTIES VERSION 1.0)
	target_link_libraries(${libname}         PRIVATE ${shared_lib_dependency_private})
	target_link_libraries(${libname}         PUBLIC ${shared_lib_dependency_public})
	target_compile_definitions(${libname}    PRIVATE X_GAME_OPTION_EXPORT_API)
endif()

install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/./include/jdc
	DESTINATION include
	FILES_MATCHING REGEX ".+\.(h|hpp)"
	)

install(
	DIRECTORY ${ROOT_PROJECT_BINARY_PATH}/../xel/.local/lib
	DESTINATION bin
	FILES_MATCHING REGEX "libxel.+"
	)

install(TARGETS
	${libname}
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION bin/lib
	)