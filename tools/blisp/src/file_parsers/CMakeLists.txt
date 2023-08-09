list(APPEND ADD_INCLUDE
"${CMAKE_CURRENT_SOURCE_DIR}/bin"
"${CMAKE_CURRENT_SOURCE_DIR}/dfu"
"${CMAKE_CURRENT_SOURCE_DIR}"
)

file(GLOB_RECURSE sources
)
list(APPEND ADD_SRCS  ${sources})

add_library(file_parsers STATIC
"${CMAKE_CURRENT_SOURCE_DIR}/bin/bin_file.c"
"${CMAKE_CURRENT_SOURCE_DIR}/dfu/dfu_file.c"
"${CMAKE_CURRENT_SOURCE_DIR}/dfu/dfu_crc.c"
"${CMAKE_CURRENT_SOURCE_DIR}/parse_file.c"
"${CMAKE_CURRENT_SOURCE_DIR}/get_file_contents.c"
)

target_include_directories(file_parsers PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}/bin
${CMAKE_CURRENT_SOURCE_DIR}/dfu
${CMAKE_CURRENT_SOURCE_DIR}

)
