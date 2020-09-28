# root

cmake_minimum_required(VERSION 2.6)

# this policy warning is due to lib command lines arg output from llvm-config
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

project (corvus)

set(LLVM_MIN_VERSION       "2006000")
set(LLVM_MIN_VERSION_TEXT  "2.6")
set(BOOST_MIN_VERSION      "103800")
set(BOOST_MIN_VERSION_TEXT "1.38")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules )
set(COR_INCLUDE_DIR ${PROJECT_SOURCE_DIR})

set(CMAKE_CXX_FLAGS "-fPIC")
#set(CMAKE_CXX_FLAGS_DEBUG "-fexceptions -g -Wall -Winline -W -Wwrite-strings -Wno-unused")

MESSAGE( STATUS "PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR} )
MESSAGE( STATUS "CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH} )

MESSAGE( STATUS "COR_INCLUDE_DIR: " ${COR_INCLUDE_DIR} )
MESSAGE( STATUS "COR_LIB_DIR: " ${COR_LIB_DIR} )
MESSAGE( STATUS "CMAKE_SYSTEM_PROCESSOR: " ${CMAKE_SYSTEM_PROCESSOR} )

Find_Package(Boost REQUIRED)

#IF( Boost_VERSION LESS ${BOOST_MIN_VERSION} )
#  MESSAGE(FATAL_ERROR "Boost version ${Boost_VERSION} too old, please install ${BOOST_MIN_VERSION_TEXT} or greater!")
#ENDIF( Boost_VERSION LESS ${BOOST_MIN_VERSION} )

set(NEEDED_Boost_VERSION 1.69.0)
find_package(Boost ${NEEDED_Boost_VERSION} REQUIRED COMPONENTS mpi serialization system filesystem)
set(Boost_LIBS ${Boost_LIBRARIES})

Find_Package(LLVM REQUIRED)
Find_Package(Sqlite3 REQUIRED)
Find_Package(CppUnit) # optional
Find_Package(Doxygen) # optional

include_directories(${COR_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${LLVM_INCLUDE_DIR})
link_directories(${LLVM_LIB_DIR} ${Boost_LIBRARY_DIRS})

set(TINYXML_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/tinyxml")
set(MD5_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/md5")
set(LEXERTL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/3rdparty")

IF( LLVM_VERSION LESS ${LLVM_MIN_VERSION} )
  MESSAGE(FATAL_ERROR "LLVM version ${LLVM_STRING_VERSION} is too old, please install ${LLVM_MIN_VERSION_TEXT} of greater!")
ENDIF( LLVM_VERSION LESS ${LLVM_MIN_VERSION} )
add_definitions(-DLLVM_VERSION=${LLVM_VERSION})

add_subdirectory(3rdparty)
add_subdirectory(corvus)
add_subdirectory(frontend)
add_subdirectory(test)

#IF(CPPUNIT_FOUND)
#  ENABLE_TESTING()
#ENDIF(CPPUNIT_FOUND)

IF(DOXYGEN_FOUND)
  SET(DOXYFILE doc/Doxyfile.corvus)
  ADD_CUSTOM_TARGET(doc ${DOXYGEN_EXECUTABLE} ${DOXYFILE} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
ENDIF(DOXYGEN_FOUND)

add_custom_target(check ${CMAKE_CURRENT_SOURCE_DIR}/test/corvus-test
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test/
                  )

# XXX this includes more than necessary
install(DIRECTORY corvus/ DESTINATION
                  "include/corvus"
                   FILES_MATCHING PATTERN "*.h"
        )
install(DIRECTORY corvus/ DESTINATION
                  "include/corvus"
                   FILES_MATCHING PATTERN "*.def"
        )
