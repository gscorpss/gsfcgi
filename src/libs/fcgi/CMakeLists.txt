include_directories(${CMAKE_SOURCE_DIR}/src/libs)

add_library(gs-fcgi SHARED
    gsfcgi.cpp
    )

target_link_libraries(gs-fcgi
    gs-sockpp
    gs-system

    )