
MESSAGE( STATUS "testing check" )

add_definitions(${LLVM_COMPILE_FLAGS})

include_directories (${CMAKE_SOURCE_DIR} 
                     ${CMAKE_BINARY_DIR} 
                    )

set(COR_TEST_FILES
    driver.cpp)

add_executable( corvus-test ${COR_TEST_FILES} )

set_target_properties(corvus-test
                      PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                     )
target_link_libraries ( corvus-test
                        libcorvus
			)
