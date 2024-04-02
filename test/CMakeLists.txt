find_package(Catch2 REQUIRED)

set(CMAKE_REQUIRED_LINK_OPTIONS "-arch;arm64")

add_executable(pcctest core.test.cpp parsers/escaped.test.cpp)

target_link_libraries(pcctest PRIVATE Catch2::Catch2WithMain)

target_include_directories(pcctest PUBLIC ${CMAKE_SOURCE_DIR})

include(Catch)

catch_discover_tests(pcctest)
