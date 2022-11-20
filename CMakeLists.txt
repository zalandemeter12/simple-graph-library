cmake_minimum_required(VERSION 3.0.0)
project(simple-graph-library VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -pedantic -Werror)

add_library(libsgl SGL/sgl.hxx)
set_target_properties(libsgl PROPERTIES LINKER_LANGUAGE CXX)

add_executable(example main.cc)
target_link_libraries(example libsgl)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
