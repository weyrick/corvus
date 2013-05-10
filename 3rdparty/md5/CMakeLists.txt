MESSAGE( STATUS "3rd party: md5 check" )

set(MD5_SRC_FILES
   md5.c
   )

add_definitions("-fPIC")

# STATIC
add_library( corvus_md5 ${MD5_SRC_FILES} )
