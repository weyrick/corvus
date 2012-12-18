# CLI frontend
MESSAGE( STATUS "CLI frontend check" )

add_definitions(${LLVM_COMPILE_FLAGS})

include_directories (${CMAKE_SOURCE_DIR} 
                     ${CMAKE_BINARY_DIR} 
                     # this one is for corvus_grammar generated files
                     ${PROJECT_BINARY_DIR}/compiler/analysis
                    )

#set(COR_SRC_FILES
#  corvus.cpp
#  ${LLVM_LIBS_JIT_OBJECTS}
#   )

#set(CORI_SRC_FILES
#  corvusi.cpp
#  ${LLVM_LIBS_JIT_OBJECTS}
#)

set(COR_ANALYSIS_FILES
    corvus-analyzer.cpp)

#add_executable( corvus ${COR_SRC_FILES} )
#add_executable( corvusi ${CORI_SRC_FILES} )
add_executable( corvus-analyzer ${COR_ANALYSIS_FILES} )

#set_target_properties( corvus
#                       PROPERTIES LINK_FLAGS ${LLVM_LDFLAGS}
#                     )
                     
#set_target_properties( corvusi
#                       PROPERTIES LINK_FLAGS ${LLVM_LDFLAGS}
#                     )

set_target_properties( corvus-analyzer
                       PROPERTIES LINK_FLAGS ${LLVM_LDFLAGS}
                     )

#target_link_libraries( corvus
#                       corvus-llvm-jit
#                       corvus-llvm-ir
#                       corvus-analysis
#                       corvus-runtime
#                       ${LLVM_LIBS_CORE}
#                       ${LLVM_LIBS_JIT}
#                       ${ICU_LIBRARIES}
#                        dl pthread
#                      )
#target_link_libraries( corvusi
#                       corvus-llvm-jit
#                       corvus-llvm-ir
#                       corvus-analysis
#                       corvus-runtime
#                       ${LLVM_LIBS_JIT}
#                       ${ICU_LIBRARIES}
#                       )                       

target_link_libraries ( corvus-analyzer
                        corvus-analysis
                        tinyxml
                       ${LLVM_LIBS_SUPPORT} 
                       ${ICU_LIBRARIES}
                        dl pthread
			)