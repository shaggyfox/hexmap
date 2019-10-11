cmake_minimum_required(VERSION 3.12.4)
add_subdirectory(ShaggysGameLib)
include_directories(${ENGINE_INCLUDE_DIRS} ShaggysGameLib ${CMAKE_BINARY_DIR})

# build .html for emscripten
if (CMAKE_C_COMPILER MATCHES "/em.*")
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

# this is helpful for YCM
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

# you may generate c-headers from any binary data
generate_data(${CMAKE_CURRENT_SOURCE_DIR}/hextile.png hextile.h hextile)
#add_executable(my_game my_game.c ${CMAKE_CURRENT_BINARY_DIR}/generated_header.h)

add_executable(hextest hextest.c hex.c hex.h ${CMAKE_CURRENT_BINARY_DIR}/hextile.h)
target_link_libraries(hextest engine)
target_compile_options(hextest PUBLIC ${ENGINE_CFLAGS})

add_executable(windows windows.c)
target_link_libraries(windows engine)
target_compile_options(windows PUBLIC ${ENGINE_CFLAGS})


