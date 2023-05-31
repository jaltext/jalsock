add_executable(${PROJECT_NAME}_test
    main.cpp
    server.cpp
)

target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})
