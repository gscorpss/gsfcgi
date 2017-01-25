
include_directories(
    ${CMAKE_SOURCE_DIR}/src/libs
    )

add_library(gs-sockpp SHARED
    Sockets.cpp
    SocketOperations.cpp
    IPv4Addr.cpp
    Epoller.cpp
    StreamBuffer.cpp
    )

target_link_libraries(gs-sockpp
    gs-system
    )