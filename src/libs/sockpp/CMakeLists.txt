
include_directories(
    ${CMAKE_SOURCE_DIR}/src/libs
    )

add_library(sockpp SHARED
    Sockets.cpp
    SocketOperations.cpp
    IPv4Addr.cpp
    Epoller.cpp
    )

target_link_libraries(sockpp
    gs-system
    )