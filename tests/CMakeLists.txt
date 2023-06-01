add_executable(${PROJECT_NAME}_server
    main_server.cpp
    server.cpp
)

target_link_libraries(${PROJECT_NAME}_server ${PROJECT_NAME})

add_executable(${PROJECT_NAME}_client
    main_client.cpp
    client.cpp
)

target_link_libraries(${PROJECT_NAME}_client ${PROJECT_NAME})
