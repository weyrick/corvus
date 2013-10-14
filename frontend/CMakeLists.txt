# CLI frontend
MESSAGE( STATUS "CLI frontend check" )

add_definitions(${LLVM_COMPILE_FLAGS})

include_directories (${CMAKE_SOURCE_DIR} 
                     ${CMAKE_BINARY_DIR} 
                    )

set(COR_ANALYSIS_FILES
    corvus.cpp)

add_executable( corvus ${COR_ANALYSIS_FILES} )

target_link_libraries ( corvus
                        libcorvus
			)

install(TARGETS corvus
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
       )
