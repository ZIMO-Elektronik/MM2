file(GLOB_RECURSE SRC *.cpp)
add_executable(MM2Decoder ${SRC})

sanitize(address,undefined)

target_common_warnings(MM2Decoder PRIVATE)

target_link_libraries(MM2Decoder PRIVATE MM2::MM2)
