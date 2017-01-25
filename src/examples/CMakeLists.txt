find_package( Threads )

include_directories(
    ${CMAKE_SOURCE_DIR}/src/libs SYSTEM
    )

# add_executable(echo
#     Echo.cpp
#     )
#
# target_link_libraries(echo
#     sockpp
#     gs-system
#     ${CMAKE_THREAD_LIBS_INIT}
#
#     )

add_executable(testpp
    test.cpp
    )

target_link_libraries(testpp
    gs-sockpp
    gs-system
    ${CMAKE_THREAD_LIBS_INIT}

    )
