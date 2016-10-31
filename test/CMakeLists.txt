find_package(GTest REQUIRED)

include_directories(
    ${GTEST_INCLUDE_DIRS} SYSTEM
    ${CMAKE_SOURCE_DIR}/src/libs SYSTEM
    )

function(AddUnitTest name src linklib)
    add_executable(${name}
        ${src}
        )

    target_link_libraries(${name}
        ${GTEST_BOTH_LIBRARIES}
        pthread
        ${linklib}
        )

    add_test(
        NAME ${name}
        COMMAND  $<TARGET_FILE:${name}>
        )
endfunction(AddUnitTest)

AddUnitTest(SocketTest SocketTest.cpp sockpp)
AddUnitTest(Ipv4AddrTest Ipv4AddrTest.cpp sockpp)
AddUnitTest(EpollerTest EpollerTest.cpp sockpp)
