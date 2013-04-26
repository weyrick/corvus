# CLI frontend
MESSAGE( STATUS "CLI frontend check" )

add_definitions(${LLVM_COMPILE_FLAGS})

include_directories (${CMAKE_SOURCE_DIR} 
                     ${CMAKE_BINARY_DIR} 
                     # this one is for corvus_grammar generated files
                     ${PROJECT_BINARY_DIR}/compiler/analysis
                    )

set(COR_ANALYSIS_FILES
    corvus-analyzer.cpp)

add_executable( corvus ${COR_ANALYSIS_FILES} )

set_target_properties( corvus
                       PROPERTIES LINK_FLAGS ${LLVM_LDFLAGS}
                     )

target_link_libraries ( corvus
                        corvus-analysis
                        tinyxml
                        corvus_md5
                        boost_system-mt
                       ${LLVM_LIBS_SUPPORT}
                        dl pthread
			)
