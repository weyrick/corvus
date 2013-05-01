MESSAGE( STATUS "libcorvus check" )

include_directories (
                     ${COR_INCLUDE_DIR}
                     ${Boost_INCLUDE_DIRS}
                     ${LLVM_INCLUDE_DIR}
                     ${MD5_INCLUDE_DIR}
                     ${LEXERTL_INCLUDE_DIR}
                     ${TINYXML_INCLUDE_DIR} # dumpast
                     # this one is for corvus_grammar generated files
                     ${CMAKE_CURRENT_BINARY_DIR}
                    )

#### PARSER ####

# lexer, parser generation
# http://www.cmake.org/pipermail/cmake/2002-September/003028.html

add_executable( lemon-pg
                grammar_src/lemon.c )

ADD_CUSTOM_COMMAND(
   # note we do this copy because lemon will output to the source directory instead of the build
   # dir if we don't, so this keeps it tidy.
   # alternative (read: portable) way is to change lemon.c to not include the base directory
   # when outputing the generating parser files, but instead strip it and use current working
   COMMAND cp
   ARGS ${CMAKE_CURRENT_SOURCE_DIR}/grammar_src/corvus_grammar.y ${CMAKE_CURRENT_BINARY_DIR}
   COMMAND cp
   ARGS ${CMAKE_CURRENT_SOURCE_DIR}/grammar_src/lempar.c ${CMAKE_CURRENT_BINARY_DIR}
   COMMAND ${CMAKE_CURRENT_BINARY_DIR}/lemon-pg
   ARGS ${CMAKE_CURRENT_BINARY_DIR}/corvus_grammar.y
   DEPENDS lemon-pg
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/grammar_src/corvus_grammar.y
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/corvus_grammar.cpp)

# Since corvus_grammar.c does not exists yet when cmake is run, mark
# it as generated
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/corvus_grammar.cpp GENERATED)

#### LEXER ####

add_executable( corvus-lexer-gen
                lexer_src/corvus-lexer-gen.cpp )

# we need corvus_grammar.h
SET_SOURCE_FILES_PROPERTIES(lexer_src/corvus-lexer-gen.cpp OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/corvus_grammar.cpp)

ADD_CUSTOM_COMMAND(
   COMMAND ${CMAKE_CURRENT_BINARY_DIR}/corvus-lexer-gen
   DEPENDS corvus-lexer-gen
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/corvus_lang_lexer.h
          ${CMAKE_CURRENT_BINARY_DIR}/corvus_dq_lexer.h
   )

# Since corvus_lexer.cpp does not exists yet when cmake is run, mark
# it as generated
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/corvus_lang_lexer.h GENERATED)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/corvus_dq_lexer.h GENERATED)
# make lexer depend on generator so it regenerates if we change it
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/corvus_lang_lexer.h OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lexer_src/corvus-lexer-gen.cpp)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/corvus_dq_lexer.h OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lexer_src/corvus-lexer-gen.cpp)

# pLexer depends on generated lexer
SET_SOURCE_FILES_PROPERTIES(pLexer.cpp OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/corvus_lang_lexer.h)

####

set(PARSER_SRC_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/corvus_grammar.cpp
  pAST.cpp
  pBaseVisitor.cpp
  #pBaseTransformer.cpp
  pParseContext.cpp
  pSourceFile.cpp
  pLexer.cpp
  pParser.cpp
  pSourceModule.cpp
  pSourceManager.cpp
  pPass.cpp
  pPassManager.cpp
  pModel.cpp
  # passes
  passes/PrintAST.cpp
  passes/DumpStats.cpp
  passes/Trivial.cpp
  passes/ModelBuilder.cpp
  passes/ModelChecker.cpp
)

# PASSES
add_definitions("-DTIXML_USE_STL")

set_source_files_properties( ${PARSER_SRC_FILES}
                             PROPERTIES COMPILE_FLAGS ${LLVM_COMPILE_FLAGS}
                            )

add_library( libcorvus ${PARSER_SRC_FILES} )
#set_target_properties(liborvus
#                      PROPERTIES
#                      OUTPUT_NAME corvus
#                     )
target_link_libraries ( libcorvus
                        tinyxml
                        corvus_md5
                        boost_system-mt
                        ${LLVM_LIBS_SUPPORT}
                        dl pthread
                        ${SQLITE3_LIBRARIES}
                        )
