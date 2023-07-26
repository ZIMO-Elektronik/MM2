file(GLOB_RECURSE SRC *.cpp)
add_executable(MM2Examples ${SRC})

sanatize("address,undefined")

target_common_warnings(MM2Examples PRIVATE)

target_link_libraries(MM2Examples PRIVATE MM2::MM2)
