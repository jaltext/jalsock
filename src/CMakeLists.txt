add_library(${PROJECT_NAME} STATIC
    socket/socket.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/include)

