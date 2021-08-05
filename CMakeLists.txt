cmake_minimum_required (VERSION 3.15)
project (file_signature)


include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package (Threads)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.45.0 COMPONENTS program_options)

enable_testing()
include_directories(${GTEST_INCLUDE_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "-pthread ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

SET (CMAKE_CXX_FLAGS_THREAD_SANITIZE       "-O0 -fsanitize=thread")
SET (CMAKE_CXX_FLAGS_ADDRESS_SANITIZE       "-O0 -fsanitize=address")
SET (CMAKE_EXE_LINKER_FLAGS_SANITIZE "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")

include_directories("./")
include_directories(${Boost_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES "file_signature/*.cpp" "file_signature/*.hpp")
file(GLOB_RECURSE ONLY_UNITTESTS_SOURCES "file_signature/*.test.cpp")
file(GLOB_RECURSE ONLY_UNITTESTS_HEADERS "file_signature/*.test.h")
file(GLOB_RECURSE UNITTESTS_SOURCES "file_signature/*.cpp" "file_signature/*.hpp")
file(GLOB_RECURSE ONLY_MAIN_SOURCE "file_signature/main.cpp")

foreach(element ${ONLY_UNITTESTS_SOURCES})
    list(REMOVE_ITEM SOURCES ${element})
endforeach()

foreach(element ${ONLY_UNITTESTS_HEADERS})
    list(REMOVE_ITEM SOURCES ${element})
endforeach()

list(REMOVE_ITEM UNITTESTS_SOURCES "${ONLY_MAIN_SOURCE}")

add_executable(file_signature  ${SOURCES})
target_link_libraries(file_signature ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

add_executable(file_signature_unit_tests  ${UNITTESTS_SOURCES})
target_link_libraries(file_signature_unit_tests gtest_main ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
add_test(NAME file_signature_unit_tests COMMAND file_signature_unit_tests)


