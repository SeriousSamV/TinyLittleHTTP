cmake_minimum_required(VERSION 3.29)
project(TinyLittleHTTP C)

set(CMAKE_C_STANDARD 23)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

include(CTest)
enable_testing()

add_subdirectory(./tiny_libs/TinyLittleURLUtils)

add_library(tiny_http_server_lib STATIC src/tiny_http/tiny_http_server_lib.c src/tiny_http/tiny_http_server_lib.h)
target_include_directories(tiny_http_server_lib PUBLIC src/tiny_http)
target_link_libraries(tiny_http_server_lib PRIVATE tiny_url_encoder_lib)

add_executable(assert_tiny_http_server_lib test/assert_tiny_http_server_lib.c)
target_link_libraries(assert_tiny_http_server_lib
        PRIVATE tiny_http_server_lib
        PRIVATE tiny_url_decoder_lib)

add_test(test_tiny_http_server_lib assert_tiny_http_server_lib)
