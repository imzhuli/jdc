project(sample_${libname})

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
)

function(add_sub_sample sub_dir_name)
	aux_source_directory(${sub_dir_name} LOCAL_SOURCE_CODE)
	add_executable(${sub_dir_name}  ${LOCAL_SOURCE_CODE})
	set_target_properties(${sub_dir_name} PROPERTIES INSTALL_RPATH "${LOCAL_RPATH}/lib")
	target_link_libraries(${sub_dir_name} PRIVATE ${libname})
	target_link_libraries(${sub_dir_name} PRIVATE ${shared_lib_dependency_private})
if (X_OPTION_STATIC)
	target_compile_definitions(${sub_dir_name} PRIVATE X_OPTION_STATIC)
endif()
if (X_GAME_OPTION_STATIC)
	target_compile_definitions(${sub_dir_name} PRIVATE X_GAME_OPTION_STATIC)
endif()
endfunction()

#test lib:

add_sub_sample(dw)

install(TARGETS
	dw
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	)
