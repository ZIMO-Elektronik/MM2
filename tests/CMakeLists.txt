include(GoogleTest)

file(GLOB_RECURSE SRC *.cpp)
add_executable(MM2Tests ${SRC})

sanitize("address,undefined")

target_common_warnings(MM2Tests PRIVATE)

cpmaddpackage("gh:google/googletest#main")

target_link_libraries(MM2Tests PRIVATE MM2::MM2 GTest::gtest GTest::gmock)

gtest_discover_tests(MM2Tests)
