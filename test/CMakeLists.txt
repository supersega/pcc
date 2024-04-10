find_package(Catch2 REQUIRED)

add_executable(
    pcctest 
    core.test.cpp 
    parsers/escaped.test.cpp 
    parsers/map.test.cpp
    parsers/one_of.test.cpp)

target_link_libraries(pcctest PRIVATE Catch2::Catch2WithMain)

target_include_directories(pcctest PUBLIC ${CMAKE_SOURCE_DIR})

include(Catch)

catch_discover_tests(pcctest)
